/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include "compilation.h"
#include "function.h"
#include "libarkbase/mem/pool_manager.h"
#include "elfio.hpp"
#include "irtoc_runtime.h"
#ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
#include "aarch64/disasm-aarch64.h"
#endif

#ifdef PANDA_COMPILER_DEBUG_INFO
#include "dwarf_builder.h"
#endif

namespace ark::irtoc {

#ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
class UsedRegistersCollector : public vixl::aarch64::Disassembler {
public:
    explicit UsedRegistersCollector(ark::ArenaAllocator *allocator) : Disassembler(allocator) {}

    RegMask &GetUsedRegs(bool isFp)
    {
        return isFp ? vregMask_ : regMask_;
    }

    static UsedRegisters CollectForCode(ArenaAllocator *allocator, Span<const uint8_t> code)
    {
        ASSERT(allocator != nullptr);
        ASSERT(!code.Empty());

        vixl::aarch64::Decoder decoder(allocator);
        UsedRegistersCollector usedRegsCollector(allocator);
        decoder.AppendVisitor(&usedRegsCollector);
        bool skipping = false;

        auto startInstr = reinterpret_cast<const vixl::aarch64::Instruction *>(code.data());
        auto endInstr = reinterpret_cast<const vixl::aarch64::Instruction *>(&(*code.end()));
        // To determine real registers usage we check each assembly instruction which has
        // destination register(s). There is a problem with handlers with `return` cause
        // there is an epilogue part with registers restoring. We need to separate both register
        // restoring and real register usage. There are some heuristics were invented to make it
        // possible, it work as follows:
        //   1) We parse assembly code in reverse mode to fast the epilogue part finding.
        //   2) For each instruction we add all its destination registers to the result set
        //      of used registers.
        //   3) When we met `ret` instruction then we raise special `skipping` flag for a few
        //      next instructions.
        //   4) When `skipping` flag is raised, while we meet `load` instructions or `add`
        //      arithmetic involving `sp` (stack pointer) as destination we continue to skip such
        //      instructions (assuming they are related to epilogue part) but without adding their
        //      registers into the result set. If we meet another kind of intruction we unset
        //      the `skipping` flag.
        for (auto instr = usedRegsCollector.GetPrevInstruction(endInstr); instr >= startInstr;
             instr = usedRegsCollector.GetPrevInstruction(instr)) {
            if (instr->Mask(vixl::aarch64::UnconditionalBranchToRegisterMask) == vixl::aarch64::RET) {
                skipping = true;
                continue;
            }
            if (skipping && (instr->IsLoad() || usedRegsCollector.CheckSPAdd(instr))) {
                continue;
            }
            skipping = false;
            decoder.Decode(instr);
        }

        UsedRegisters usedRegisters;
        usedRegisters.gpr |= usedRegsCollector.GetUsedRegs(false);
        usedRegisters.fp |= usedRegsCollector.GetUsedRegs(true);
        return usedRegisters;
    }

protected:
    const vixl::aarch64::Instruction *GetPrevInstruction(const vixl::aarch64::Instruction *instr) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return instr - vixl::aarch64::kInstructionSize;
    }

    bool CheckSPAdd(const vixl::aarch64::Instruction *instr) const
    {
        constexpr int32_t SP_REG = GetDwarfSP(Arch::AARCH64);
        return instr->Mask(vixl::aarch64::AddSubOpMask) == vixl::aarch64::ADD && (instr->GetRd() == SP_REG);
    }

    void AppendRegisterNameToOutput(const vixl::aarch64::Instruction *instr,
                                    const vixl::aarch64::CPURegister &reg) override
    {
        Disassembler::AppendRegisterNameToOutput(instr, reg);
        if (instr->IsStore()) {
            return;
        }
        uint32_t code = reg.GetCode();
        // We need to account for both registers in case of a pair load
        bool isPair = instr->Mask(vixl::aarch64::LoadStorePairAnyFMask) == vixl::aarch64::LoadStorePairAnyFixed;
        if (!(code == static_cast<uint32_t>(instr->GetRd()) ||
              (isPair && code == static_cast<uint32_t>(instr->GetRt2())))) {
            return;
        }
        if (reg.IsRegister()) {
            if (!reg.IsZero()) {
                regMask_.Set(code);
            }
        } else {
            ASSERT(reg.IsVRegister());
            vregMask_.Set(code);
        }
    }

private:
    RegMask regMask_;
    VRegMask vregMask_;
};
#endif  // ifdef LLVM_INTERPRETER_CHECK_REGS_MASK

// elfio library missed some elf constants, so lets define it here for a while. We can't include elf.h header because
// it conflicts with elfio.
static constexpr size_t EF_ARM_EABI_VER5 = 0x05000000;

void Compilation::CollectUsedRegisters([[maybe_unused]] ark::ArenaAllocator *allocator)
{
#ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
    if (arch_ == Arch::AARCH64) {
        for (auto unit : units_) {
            if ((unit->GetGraph()->GetMode().IsInterpreter() || unit->GetGraph()->GetMode().IsInterpreterEntry()) &&
                unit->GetCompilationResult() != CompilationResult::ARK) {
                usedRegisters_ |= UsedRegistersCollector::CollectForCode(allocator, unit->GetCode());
            }
        }
    }
#endif  // ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
}

void Compilation::CheckUsedRegisters()
{
#ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
    if (usedRegisters_.gpr.Count() > 0) {
        LOG(INFO, IRTOC) << "LLVM Irtoc compilation: used registers " << usedRegisters_.gpr;
        usedRegisters_.gpr &= GetCalleeRegsMask(arch_, false);
        auto diff = usedRegisters_.gpr ^ GetCalleeRegsMask(arch_, false, true);
        if (diff.Any()) {
            LOG(FATAL, IRTOC) << "LLVM Irtoc compilation callee saved register usage is different from optimized set"
                              << std::endl
                              << "Expected: " << GetCalleeRegsMask(arch_, false, true) << std::endl
                              << "Got: " << usedRegisters_.gpr;
        }
    }
    if (usedRegisters_.fp.Count() > 0) {
        LOG(INFO, IRTOC) << "LLVM Irtoc compilation: used fp registers " << usedRegisters_.fp;
        usedRegisters_.fp &= GetCalleeRegsMask(arch_, true);
        auto diff = usedRegisters_.fp ^ GetCalleeRegsMask(arch_, true, true);
        if (diff.Any()) {
            LOG(FATAL, IRTOC) << "LLVM Irtoc compilation callee saved fp register usage is different from optimized set"
                              << std::endl
                              << "Expected: " << GetCalleeRegsMask(arch_, true, true) << std::endl
                              << "Got: " << usedRegisters_.fp;
        }
    }
#endif
}

Compilation::Result Compilation::Run()
{
    if (compiler::g_options.WasSetCompilerRegexWithSignature()) {
        LOG(FATAL, IRTOC) << "Regex with signatures is not supported, please use '--compiler-regex'.";
    }
    if (compiler::g_options.WasSetCompilerRegex()) {
        methodsRegex_ = compiler::g_options.GetCompilerRegex();
    }

    PoolManager::Initialize(PoolType::MALLOC);

    allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);
    localAllocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);

    if (RUNTIME_ARCH == Arch::X86_64 && compiler::g_options.WasSetCompilerCrossArch()) {
        arch_ = GetArchFromString(compiler::g_options.GetCompilerCrossArch());
        if (arch_ == Arch::NONE) {
            LOG(FATAL, IRTOC) << "FATAL: unknown arch: " << compiler::g_options.GetCompilerCrossArch();
        }
        compiler::g_options.AdjustCpuFeatures(arch_ != RUNTIME_ARCH);
    } else {
        compiler::g_options.AdjustCpuFeatures(false);
    }

    LOG(INFO, IRTOC) << "Start Irtoc compilation for " << GetArchString(arch_) << "...";

    auto result = Compile();
    if (result) {
        CheckUsedRegisters();
        LOG(INFO, IRTOC) << "Irtoc compilation success";
    } else {
        LOG(FATAL, IRTOC) << "Irtoc compilation failed: " << result.Error();
    }

    if (result = MakeElf(g_options.GetIrtocOutput()); !result) {
        return result;
    }

    for (auto unit : units_) {
        delete unit;
    }

    allocator_.reset();
    localAllocator_.reset();

    PoolManager::Finalize();

    return result;
}

Compilation::Result Compilation::Compile()
{
#ifdef PANDA_LLVM_IRTOC
    IrtocRuntimeInterface runtime;
    ArenaAllocator allocator(SpaceType::SPACE_TYPE_COMPILER);
    std::shared_ptr<llvmbackend::IrtocCompilerInterface> llvmCompiler =
        llvmbackend::CreateLLVMIrtocCompiler(&runtime, &allocator, arch_);
#endif

    for (auto unit : units_) {
        if (compiler::g_options.WasSetCompilerRegex() && !std::regex_match(unit->GetName(), methodsRegex_)) {
            continue;
        }
        LOG(INFO, IRTOC) << "Compile " << unit->GetName();
#ifdef PANDA_LLVM_IRTOC
        unit->SetLLVMCompiler(llvmCompiler);
#endif
        auto result = unit->Compile(arch_, allocator_.get(), localAllocator_.get());
        if (!result) {
            return Unexpected {result.Error()};
        }
#ifdef PANDA_COMPILER_DEBUG_INFO
        hasDebugInfo_ |= unit->GetGraph()->IsLineDebugInfoEnabled();
#endif
    }

#ifdef PANDA_LLVM_IRTOC
    llvmCompiler->FinishCompile();
    ASSERT(!g_options.GetIrtocOutputLlvm().empty());
    llvmCompiler->WriteObjectFile(g_options.GetIrtocOutputLlvm());

    for (auto unit : units_) {
        if (unit->IsCompiledByLlvm()) {
            auto code = llvmCompiler->GetCompiledCode(unit->GetName());
            Span<uint8_t> span = {const_cast<uint8_t *>(code.code), code.size};
            unit->SetCode(span);
        }
        unit->ReportCompilationStatistic(&std::cerr);
    }
    if (g_options.GetIrtocLlvmStats() != "none" && !llvmCompiler->IsEmpty()) {
        std::cerr << "LLVM total: " << llvmCompiler->GetObjectFileSize() << " bytes" << std::endl;
    }

#ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
    CollectUsedRegisters(&allocator);
#endif
#endif  // PANDA_LLVM_IRTOC

    return 0;
}

static size_t GetElfArch(Arch arch)
{
    switch (arch) {
        case Arch::AARCH32:
            return ELFIO::EM_ARM;
        case Arch::AARCH64:
            return ELFIO::EM_AARCH64;
        case Arch::X86:
            return ELFIO::EM_386;
        case Arch::X86_64:
            return ELFIO::EM_X86_64;
        default:
            UNREACHABLE();
    }
}

// CC-OFFNXT(huge_method[C++],G.FUN.01-CPP) solid logic
// CODECHECK-NOLINTNEXTLINE(C_RULE_ID_FUNCTION_SIZE)
Compilation::Result Compilation::MakeElf(std::string_view output)
{
    ELFIO::elfio elfWriter;
    elfWriter.create(Is64BitsArch(arch_) ? ELFIO::ELFCLASS64 : ELFIO::ELFCLASS32, ELFIO::ELFDATA2LSB);
    elfWriter.set_type(ELFIO::ET_REL);
    if (arch_ == Arch::AARCH32) {
        elfWriter.set_flags(EF_ARM_EABI_VER5);
    }
    elfWriter.set_os_abi(ELFIO::ELFOSABI_NONE);
    elfWriter.set_machine(GetElfArch(arch_));

    ELFIO::section *strSec = elfWriter.sections.add(".strtab");
    strSec->set_type(ELFIO::SHT_STRTAB);
    strSec->set_addr_align(0x1);

    ELFIO::string_section_accessor strWriter(strSec);

    static constexpr size_t FIRST_GLOBAL_SYMBOL_INDEX = 2;
    static constexpr size_t SYMTAB_ADDR_ALIGN = 8;

    ELFIO::section *symSec = elfWriter.sections.add(".symtab");
    symSec->set_type(ELFIO::SHT_SYMTAB);
    symSec->set_info(FIRST_GLOBAL_SYMBOL_INDEX);
    symSec->set_link(strSec->get_index());
    symSec->set_addr_align(SYMTAB_ADDR_ALIGN);
    symSec->set_entry_size(elfWriter.get_default_entry_size(ELFIO::SHT_SYMTAB));

    ELFIO::symbol_section_accessor symbolWriter(elfWriter, symSec);

    symbolWriter.add_symbol(strWriter, "irtoc.cpp", 0, 0, ELFIO::STB_LOCAL, ELFIO::STT_FILE, 0, ELFIO::SHN_ABS);

    ELFIO::section *textSec = elfWriter.sections.add(".text");
    textSec->set_type(ELFIO::SHT_PROGBITS);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    textSec->set_flags(ELFIO::SHF_ALLOC | ELFIO::SHF_EXECINSTR);
    textSec->set_addr_align(GetCodeAlignment(arch_));

    ELFIO::section *relSec = elfWriter.sections.add(".rela.text");
    relSec->set_type(ELFIO::SHT_RELA);
    relSec->set_info(textSec->get_index());
    relSec->set_link(symSec->get_index());
    relSec->set_addr_align(4U);  // CODECHECK-NOLINT(C_RULE_ID_MAGICNUMBER)
    relSec->set_entry_size(elfWriter.get_default_entry_size(ELFIO::SHT_RELA));
    ELFIO::relocation_section_accessor relWriter(elfWriter, relSec);

    /* Use symbols map to avoid saving the same symbols multiple times */
    std::unordered_map<std::string, uint32_t> symbolsMap;
    auto addSymbol = [&symbolsMap, &symbolWriter, &strWriter](const char *name) {
        if (auto it = symbolsMap.find(name); it != symbolsMap.end()) {
            return it->second;
        }
        uint32_t index = symbolWriter.add_symbol(strWriter, name, 0, 0, ELFIO::STB_GLOBAL, ELFIO::STT_NOTYPE, 0, 0);
        symbolsMap.insert({name, index});
        return index;
    };
#ifdef PANDA_COMPILER_DEBUG_INFO
    auto dwarfBuilder {hasDebugInfo_ ? std::make_optional<DwarfBuilder>(arch_, &elfWriter) : std::nullopt};
#endif

    static constexpr size_t MAX_CODE_ALIGNMENT = 64;
    static constexpr std::array<uint8_t, MAX_CODE_ALIGNMENT> PADDING_DATA {0};
    CHECK_LE(GetCodeAlignment(GetArch()), MAX_CODE_ALIGNMENT);

    uint32_t codeAlignment = GetCodeAlignment(GetArch());
    ASSERT(codeAlignment != 0);
    size_t offset = 0;
    for (auto unit : units_) {
        if (unit->IsCompiledByLlvm()) {
            continue;
        }
        auto code = unit->GetCode();

        // Align function
        if (auto padding = (codeAlignment - (offset % codeAlignment)) % codeAlignment; padding != 0) {
            textSec->append_data(reinterpret_cast<const char *>(PADDING_DATA.data()), padding);
            offset += padding;
        }
        ASSERT(offset % codeAlignment == 0);

        auto symbol = symbolWriter.add_symbol(strWriter, unit->GetName(), offset, code.size(), ELFIO::STB_GLOBAL,
                                              ELFIO::STT_FUNC, 0, textSec->get_index());
        (void)symbol;
        textSec->append_data(reinterpret_cast<const char *>(code.data()), code.size());
        for (auto &rel : unit->GetRelocations()) {
            size_t relOffset = offset + rel.offset;
            auto sindex = static_cast<uint64_t>(addSymbol(unit->GetExternalFunction(rel.data)));
            if (Is64BitsArch(arch_)) {
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                relWriter.add_entry(relOffset, static_cast<ELFIO::Elf_Xword>(ELF64_R_INFO(sindex, rel.type)),
                                    rel.addend);
            } else {
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                relWriter.add_entry(relOffset, static_cast<ELFIO::Elf_Xword>(ELF32_R_INFO(sindex, rel.type)),
                                    rel.addend);
            }
        }
#ifdef PANDA_COMPILER_DEBUG_INFO
        ASSERT(!unit->GetGraph()->IsLineDebugInfoEnabled() || dwarfBuilder);
        if (dwarfBuilder && !dwarfBuilder->BuildGraph(unit, offset, symbol)) {
            return Unexpected("DwarfBuilder::BuildGraph failed!");
        }
#endif
        offset += code.size();
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (dwarfBuilder && !dwarfBuilder->Finalize(offset)) {
        return Unexpected("DwarfBuilder::Finalize failed!");
    }
#endif

    elfWriter.save(output.data());

    return 0;
}
}  // namespace ark::irtoc
