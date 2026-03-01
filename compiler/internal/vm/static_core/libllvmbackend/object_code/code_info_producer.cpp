/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "compiler/code_info/code_info_builder.h"
#include "compiler/optimizer/code_generator/target_info.h"
#include "compiler/optimizer/ir/inst.h"

#include "llvm_ark_interface.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "code_info_producer.h"

static constexpr unsigned LOCATION_STEP = 2U;
static constexpr unsigned START_OFFSET = 3U;
static constexpr unsigned DWARF_AARCH64_BASE_REG = 19U;
[[maybe_unused]] static constexpr unsigned DWARF_MAX_REG = 32U;

namespace ark::llvmbackend {

CodeInfoProducer::CodeInfoProducer(Arch arch, LLVMArkInterface *compilation) : arch_(arch), compilation_(compilation) {}

/// Sets a LLVM stack map if any
void CodeInfoProducer::SetStackMap(const uint8_t *section, uintptr_t size)
{
    stackmap_ = std::make_unique<LLVMStackMap>(llvm::makeArrayRef(section, size));
    if (g_options.IsLlvmDumpStackmaps()) {
        std::ofstream fout("llvm-stackmaps.txt");
        LLVM_LOG_IF(!fout.is_open(), FATAL, STACKMAPS) << "Couldn't open llvm-stackmaps.txt";
        DumpStackMap(stackmap_, fout);
        fout.close();
        LLVM_LOG(DEBUG, STACKMAPS) << "LLVM Stackmaps has been dumped into llvm-stackmaps.txt";
    }
}

void CodeInfoProducer::SetFaultMaps(const uint8_t *section, uintptr_t size)
{
    Span span {section, size};
    faultMapParser_ = std::make_unique<llvm::FaultMapParser>(span.begin(), span.end());
}

void CodeInfoProducer::AddSymbol(Method *method, StackMapSymbol symbol)
{
    symbols_.insert({method, symbol});
}

void CodeInfoProducer::AddFaultMapSymbol(Method *method, uint32_t symbol)
{
    faultMapSymbols_.insert({method, symbol});
}

/// Fill a CodeInfoBuilder with proper data for the passed METHOD.
void CodeInfoProducer::Produce(Method *method, compiler::CodeInfoBuilder *builder) const
{
    builder->BeginMethod(0, compilation_->GetVirtualRegistersCount(method));

    // Create empty stackmap for the stack overflow check
    compiler::SaveStateInst ss(compiler::Opcode::SaveState, 0, nullptr, nullptr, 0);
    builder->BeginStackMap(0, 0, &ss, false);
    builder->EndStackMap();

    ConvertStackMaps(method, builder);
    EncodeNullChecks(method, builder);

    auto xregsMask = GetCalleeRegsMask(arch_, false).GetValue();
    auto vregsMask = GetCalleeRegsMask(arch_, true).GetValue();

    if (arch_ == Arch::AARCH64) {
        auto funcName = compilation_->GetUniqMethodName(method);
        auto regMasks = compilation_->GetCalleeSavedRegistersMask(funcName);
        xregsMask &= std::get<0>(regMasks);
        vregsMask &= std::get<1>(regMasks);
    }
    builder->SetSavedCalleeRegsMask(xregsMask, vregsMask);
    builder->EndMethod();
}

uint16_t CodeInfoProducer::GetDwarfBase(Arch arch) const
{
    switch (arch) {
        case Arch::AARCH64:
            return DWARF_AARCH64_BASE_REG;
        default:
            LLVM_LOG(FATAL, STACKMAPS) << "Unsupported base register for " << GetIsaName(arch);
            return std::numeric_limits<uint16_t>::max();
    }
}

size_t CodeInfoProducer::GetArkFrameSlot(const LLVMStackMap::LocationAccessor &location, uint64_t stackSize,
                                         size_t slotSize) const
{
    ASSERT(slotSize != 0);
    int32_t offset = -location.getOffset();
    auto reg = location.getDwarfRegNum();
    if (reg == GetDwarfSP(arch_)) {
        LLVM_LOG(DEBUG, STACKMAPS) << "SP offset, stack size = " << stackSize;
        offset += stackSize;
        if (arch_ == Arch::X86_64) {
            // x86_64 SP points to the last inserted value
            offset -= slotSize;
        }
        if (arch_ == Arch::AARCH64) {
            // LLVM allocate slots for LR & FP, but we already save
            // it via INLINEASM, so subtract addition stack size
            offset -= 2U * slotSize;
        }
    } else if (reg == GetDwarfFP(arch_)) {
        LLVM_LOG(DEBUG, STACKMAPS) << "FP offset";
    } else if (reg == GetDwarfBase(arch_)) {
        LLVM_LOG(DEBUG, STACKMAPS) << "Base offset";
        offset += stackSize;
    } else {
        LLVM_LOG(FATAL, STACKMAPS) << "Unsupported location register: " << reg;
    }

    ASSERT_PRINT(offset % slotSize == 0, "By some reason offset is not an integer number of slots");

    auto slot = offset / slotSize;
    // Slot is starting from callee regs
    slot -= CFrameLayout::CALLEE_REGS_START_SLOT;
    LLVM_LOG(DEBUG, STACKMAPS) << "\tReg: " << location.getDwarfRegNum() << ", offset: " << location.getOffset()
                               << ", LLVM stack offset: 0x" << std::hex << offset << ", ARK stack offset: 0x" << offset
                               << std::dec << ", ARK stack slot: " << slot;
    return slot;
}

unsigned CodeInfoProducer::CollectRoots(const LLVMStackMap::RecordAccessor &record, uint64_t stackSize,
                                        ArenaBitVector *stack) const
{
    CFrameLayout fl(arch_, 0);
    unsigned regMask = 0;
    auto deoptCount = record.getLocation(LOCATION_DEOPT_COUNT).getSmallConstant();
    for (uint64_t i = LOCATION_DEOPT_COUNT + deoptCount + 1; i < record.getNumLocations(); i += LOCATION_STEP) {
        const auto &loc = record.getLocation(i);
        auto kind = loc.getKind();
        // We expect spilled references or constant null pointers that may come from "deopt" bundle
        ASSERT(kind == LLVMStackMap::LocationKind::Indirect || kind == LLVMStackMap::LocationKind::Register ||
               (kind == LLVMStackMap::LocationKind::Constant && loc.getSmallConstant() == 0));
        // Spilled value
        if (kind == LLVMStackMap::LocationKind::Indirect) {
            const auto callerRegsSlotStart = fl.GetCallerFirstSlot(false);
            const auto callerRegsCount = fl.GetCallerRegistersCount(false);
            auto slot = GetArkFrameSlot(loc, stackSize, fl.GetSlotSize());
            auto callerRegId = slot - callerRegsSlotStart;
            if (slot < fl.GetFirstSpillSlot()) {
                regMask |= (1U << (callerRegsCount - 1 - callerRegId));
            } else {
                stack->SetBit(slot - fl.GetFirstSpillSlot());
            }
        } else if (kind == LLVMStackMap::LocationKind::Register) {
            unsigned reg = loc.getDwarfRegNum();
            ASSERT(reg < DWARF_MAX_REG);
            reg = arch_ == Arch::X86_64 ? LLVMArkInterface::X86RegNumberConvert(reg) : reg;
            regMask |= (1U << reg);
        }
    }
    LLVM_LOG(DEBUG, STACKMAPS) << "Register mask:" << regMask;
    ASSERT((regMask & ~(GetCalleeRegsMask(arch_, false).GetValue() | GetCallerRegsMask(arch_, false).GetValue())) == 0);
    return regMask;
}

void CodeInfoProducer::BuildSingleRegMap(compiler::CodeInfoBuilder *builder, const LLVMStackMap::RecordAccessor &record,
                                         int32_t methodIdIndex, int32_t vregsCount, uint64_t stackSize) const
{
    int32_t vregsTotal = record.getLocation(methodIdIndex + INLINE_VREG_COUNT).getSmallConstant();
    std::vector<int> ordered;
    ordered.resize(vregsTotal, -1);
    for (auto i = 0; i < vregsCount * VREG_RECORD_SIZE; i += VREG_RECORD_SIZE) {
        int32_t vregIndex = record.getLocation(methodIdIndex + INLINE_VREGS + i + VREG_IDX).getSmallConstant();
        ordered.at(vregIndex) = i;
    }

    CFrameLayout fl(arch_, 0);
    for (auto idx : ordered) {
        if (idx == -1) {
            builder->AddVReg(compiler::VRegInfo());
            continue;
        }
        auto typeVal = record.getLocation(methodIdIndex + INLINE_VREGS + idx + VREG_TYPE).getSmallConstant();
        auto vregType = static_cast<compiler::VRegInfo::Type>(typeVal);
        const auto &loc = record.getLocation(methodIdIndex + INLINE_VREGS + idx + VREG_VALUE);
        bool isAcc = ((idx / 3) == (vregsTotal - 1));
        auto vregVregType = (isAcc ? compiler::VRegInfo::VRegType::ACC : compiler::VRegInfo::VRegType::VREG);
        if (loc.getKind() == LLVMStackMap::LocationKind::Constant) {
            int32_t constVal = loc.getSmallConstant();  // sign extend required for VRegInfo::Type::INT64
            builder->AddConstant(static_cast<int64_t>(constVal), vregType, vregVregType);
        } else if (loc.getKind() == LLVMStackMap::LocationKind::ConstantIndex) {
            uint64_t constVal = stackmap_->getConstant(loc.getConstantIndex()).getValue();
            builder->AddConstant(constVal, vregType, vregVregType);
        } else if (loc.getKind() == LLVMStackMap::LocationKind::Indirect) {
            auto slot = GetArkFrameSlot(loc, stackSize, fl.GetSlotSize());
            compiler::VRegInfo::Location vregLoc;
            int32_t value;
            if (slot <= fl.GetCalleeLastSlot(false)) {
                vregLoc = compiler::VRegInfo::Location::REGISTER;
                value = GetFirstCalleeReg(arch_, false) + fl.GetCalleeLastSlot(false) - slot;
            } else if (slot <= fl.GetCalleeLastSlot(true)) {
                vregLoc = compiler::VRegInfo::Location::FP_REGISTER;
                value = GetFirstCalleeReg(arch_, true) + fl.GetCalleeLastSlot(true) - slot;
            } else {
                vregLoc = compiler::VRegInfo::Location::SLOT;
                value = slot;
            }
            ASSERT(value > 0);
            builder->AddVReg(compiler::VRegInfo(value, vregLoc, vregType, vregVregType));
        } else if (loc.getKind() == LLVMStackMap::LocationKind::Register) {
            unsigned reg = loc.getDwarfRegNum();
            reg = arch_ == Arch::X86_64 ? LLVMArkInterface::X86RegNumberConvert(reg) : reg;
            builder->AddVReg(compiler::VRegInfo(reg, compiler::VRegInfo::Location::REGISTER, vregType, vregVregType));
        } else {
            UNREACHABLE();
        }
    }
}

void CodeInfoProducer::BuildRegMap(compiler::CodeInfoBuilder *builder, const LLVMStackMap::RecordAccessor &record,
                                   uint64_t stackSize) const
{
    auto deoptCount = record.getLocation(LOCATION_DEOPT_COUNT).getSmallConstant();
    if (deoptCount == 0) {
        return;
    }

    auto inlineDepth = record.getLocation(LOCATION_INLINE_DEPTH).getSmallConstant();

    auto inlineIndex = 0;
    auto methodIdIndex = START_OFFSET + record.getLocation(LOCATION_INLINE_TABLE + inlineIndex).getSmallConstant();
    uint32_t methodId;
    uint32_t bpc;

    uint32_t nextStart;
    if (inlineDepth > 1) {
        nextStart = START_OFFSET + record.getLocation(LOCATION_INLINE_TABLE + inlineIndex + 1).getSmallConstant();
    } else {
        nextStart = START_OFFSET + deoptCount;
    }
    uint32_t vregSlots = nextStart - methodIdIndex - INLINE_VREGS;
    ASSERT(vregSlots % VREG_RECORD_SIZE == 0);
    auto vregs = vregSlots / VREG_RECORD_SIZE;

    if (record.getLocation(methodIdIndex + INLINE_NEED_REGMAP).getSmallConstant() > 0) {
        BuildSingleRegMap(builder, record, methodIdIndex, vregs, stackSize);
    }
    for (uint32_t i = 1; i < inlineDepth; ++i) {
        inlineIndex++;
        methodIdIndex = START_OFFSET + record.getLocation(LOCATION_INLINE_TABLE + inlineIndex).getSmallConstant();
        methodId = record.getLocation(methodIdIndex + INLINE_METHOD_ID).getSmallConstant();
        bpc = record.getLocation(methodIdIndex + INLINE_BPC).getSmallConstant();
        auto vregsTotal = record.getLocation(methodIdIndex + INLINE_VREG_COUNT).getSmallConstant();
        if (i < inlineDepth - 1) {
            nextStart = START_OFFSET + record.getLocation(LOCATION_INLINE_TABLE + inlineIndex + 1).getSmallConstant();
        } else {
            nextStart = START_OFFSET + deoptCount;
        }
        vregSlots = nextStart - methodIdIndex - INLINE_VREGS;
        ASSERT(vregSlots % VREG_RECORD_SIZE == 0);
        vregs = vregSlots / VREG_RECORD_SIZE;

        builder->BeginInlineInfo(nullptr, methodId, bpc, vregsTotal);
        if (vregsTotal > 0) {
            BuildSingleRegMap(builder, record, methodIdIndex, vregs, stackSize);
        }
        builder->EndInlineInfo();
    }
}

void CodeInfoProducer::ConvertStackMaps(Method *method, CodeInfoBuilder *builder) const
{
    if (symbols_.find(method) == symbols_.end()) {
        return;
    }
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    const auto &symbol = symbols_.at(method);
    ASSERT(stackmap_ != nullptr);
    int firstRecord = 0;
    const auto &func = stackmap_->getFunction(symbol.idx);
    int nRecords = func.getRecordCount();
    for (uint32_t i = 0; i < symbol.idx; ++i) {
        firstRecord += stackmap_->getFunction(i).getRecordCount();
    }
    LLVM_LOG(DEBUG, STACKMAPS) << "# Method '" << compilation_->GetRuntime()->GetMethodFullName(method, true)
                               << "' covers records in interval [" << firstRecord << ", "
                               << (firstRecord + nRecords - 1) << "] (inclusive)";
    uint64_t stackSize = compilation_->GetMethodStackSize(method);
    for (int i = firstRecord; i < firstRecord + nRecords; ++i) {
        ArenaBitVector stackRoots(&allocator);

        const auto &record = stackmap_->getRecord(i);
        auto npc = symbol.sectionOffset + record.getInstructionOffset();
        LLVM_LOG(DEBUG, STACKMAPS) << "Statepoint ID: " << record.getID() << std::hex << ", LLVM instruction offset: 0x"
                                   << record.getInstructionOffset() << ", NPC offset: 0x" << npc << std::dec;
        LLVM_LOG(DEBUG, STACKMAPS) << "Roots:";
        auto regMap = CollectRoots(record, stackSize, &stackRoots);

        auto hasDeopts = record.getLocation(LOCATION_DEOPT_COUNT).getSmallConstant() > 0;
        auto needRegmap = false;
        uint32_t bpc;
        if (hasDeopts) {
            auto methodIdIndex = 3 + record.getLocation(LOCATION_INLINE_TABLE).getSmallConstant();
            bpc = record.getLocation(methodIdIndex + INLINE_BPC).getSmallConstant();
            needRegmap = record.getLocation(methodIdIndex + INLINE_NEED_REGMAP).getSmallConstant() > 0;
        } else {
            bpc = record.getID();
        }
        compiler::SaveStateInst ss(compiler::Opcode::SaveState, 0, nullptr, nullptr, 0);
        ss.SetRootsStackMask(&stackRoots);
        ss.SetRootsRegsMask(regMap);
        builder->BeginStackMap(bpc, npc, &ss, needRegmap);

        LLVM_LOG(DEBUG, STACKMAPS) << "RegMap:";
        BuildRegMap(builder, record, stackSize);

        builder->EndStackMap();
    }
}

void CodeInfoProducer::EncodeNullChecks(Method *method, CodeInfoBuilder *builder) const
{
    if (faultMapParser_ == nullptr) {
        return;
    }
    auto methodName = compilation_->GetUniqMethodName(method);

    auto symbol = faultMapSymbols_.find(method);
    if (symbol == faultMapSymbols_.end()) {
        return;
    }

    // FunctionInfos are stored in a list, 'symbol->second' is an index in that list
    ASSERT(faultMapParser_->getNumFunctions() > 0);
    llvm::FaultMapParser::FunctionInfoAccessor functionInfo = faultMapParser_->getFirstFunctionInfo();
    for (uint32_t i = 0; i < symbol->second; i++) {
        functionInfo = functionInfo.getNextFunctionInfo();
    }
    // Process selected functionInfo
    auto faultingPcs = functionInfo.getNumFaultingPCs();
    for (size_t i = 0; i < faultingPcs; i++) {
        auto faultInfo = functionInfo.getFunctionFaultInfoAt(i);
        unsigned instructionNativePc = faultInfo.getFaultingPCOffset();
        LLVM_LOG(DEBUG, STACKMAPS) << "Encoded implicit null check for '" << methodName << "', instructionNativePc = '"
                                   << instructionNativePc << "'";
        static constexpr uint32_t LLVM_HANDLER_TAG = 1U << 31U;
        builder->AddImplicitNullCheck(instructionNativePc, LLVM_HANDLER_TAG | faultInfo.getHandlerPCOffset());
    }
}
}  // namespace ark::llvmbackend
