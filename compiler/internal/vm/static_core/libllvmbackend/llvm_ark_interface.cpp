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

#include "compiler/optimizer/ir/runtime_interface.h"
#include "compiler/optimizer/ir/aot_data.h"
#include "runtime/include/method.h"
#include "libarkbase/events/events.h"
#include "compiler_options.h"
#include "libarkbase/utils/cframe_layout.h"
#include "llvm_ark_interface.h"
#include "llvm_logger.h"
#include "utils.h"

#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm_options.h"

#include "aot/aot_builder/aot_builder.h"

using ark::compiler::AotBuilder;
using ark::compiler::AotData;
using ark::compiler::RuntimeInterface;

static constexpr auto MEMCPY_BIG_SIZE_FOR_X86_64_SSE = 128;
static constexpr auto MEMCPY_BIG_SIZE_FOR_X86_64_NO_SSE = 64;
static constexpr auto MEMCPY_BIG_SIZE_FOR_AARCH64 = 1024;

static constexpr auto MEMSET_SMALL_SIZE_FOR_AARCH64 = 1024;
static constexpr auto MEMSET_SMALL_SIZE_FOR_X86_64_SSE = 512;
static constexpr auto MEMSET_SMALL_SIZE_FOR_X86_64_NO_SSE = 256;

static constexpr auto MEMMOVE_SMALL_SIZE_FOR_AARCH64 = 30;
static constexpr auto MEMMOVE_BIG_SIZE_FOR_AARCH64 = 64;
static constexpr auto MEMMOVE_SMALL_SIZE_FOR_X86_64_SSE = 94;
static constexpr auto MEMMOVE_BIG_SIZE_FOR_X86_64_SSE = 128;
static constexpr auto MEMMOVE_SMALL_SIZE_FOR_X86_64_NO_SSE = 48;
static constexpr auto MEMMOVE_BIG_SIZE_FOR_X86_64_NO_SSE = 64;

static constexpr auto X86_THREAD_REG = 15;
static constexpr auto AARCH64_THREAD_REG = 28U;

namespace {
constexpr ark::Arch LLVMArchToArkArch(llvm::Triple::ArchType arch)
{
    switch (arch) {
        case llvm::Triple::ArchType::aarch64:
            return ark::Arch::AARCH64;
        case llvm::Triple::ArchType::x86_64:
            return ark::Arch::X86_64;
        case llvm::Triple::ArchType::x86:
            return ark::Arch::X86;
        case llvm::Triple::ArchType::arm:
            return ark::Arch::AARCH32;
        default:
            UNREACHABLE();
            return ark::Arch::NONE;
    }
}
#include <entrypoints_gen.inl>
#include <intrinsics_gen.inl>
#include <intrinsic_names_gen.inl>
#include <entrypoints_llvm_ark_interface_gen.inl>
}  // namespace

namespace ark::llvmbackend {

bool LLVMArkInterface::DeoptsEnabled()
{
    return g_options.IsLlvmDeopts();
}

AotData GetAotDataFromBuilder(const class ark::panda_file::File *file, AotBuilder *aotBuilder)
{
    return AotData(AotData::AotDataArgs {file,
                                         nullptr,
                                         nullptr,
                                         0,
                                         aotBuilder->GetIntfInlineCacheIndex(),
                                         {aotBuilder->GetGotPlt(), aotBuilder->GetGotVirtIndexes(),
                                          aotBuilder->GetGotClass(), aotBuilder->GetGotString()},
                                         {aotBuilder->GetGotIntfInlineCache(), aotBuilder->GetGotCommon()}});
}

ark::llvmbackend::LLVMArkInterface::LLVMArkInterface(RuntimeInterface *runtime, llvm::Triple triple,
                                                     AotBuilder *aotBuilder, llvm::sys::Mutex *lock)
    : runtime_(runtime), triple_(std::move(triple)), aotBuilder_(aotBuilder), lock_ {lock}
{
    ASSERT(runtime != nullptr);
}

intptr_t LLVMArkInterface::GetCompiledEntryPointOffset() const
{
    return runtime_->GetCompiledEntryPointOffset(LLVMArchToArkArch(triple_.getArch()));
}

const char *LLVMArkInterface::GetThreadRegister() const
{
    switch (LLVMArchToArkArch(triple_.getArch())) {
        case Arch::AARCH64: {
            static_assert(ArchTraits<Arch::AARCH64>::THREAD_REG == AARCH64_THREAD_REG);
            return "x28";
        }
        case Arch::X86_64: {
            static_assert(ArchTraits<Arch::X86_64>::THREAD_REG == X86_THREAD_REG);
            return "r15";
        }
        default:
            UNREACHABLE();
    }
}

int32_t LLVMArkInterface::CreateIntfInlineCacheSlotId(const llvm::Function *caller) const
{
    llvm::sys::ScopedLock scopedLock {*lock_};
    auto callerName = caller->getName();
    auto callerOriginFile = functionOrigins_.lookup(callerName);
    ASSERT_PRINT(callerOriginFile != nullptr,
                 std::string("No origin for function = '") + callerName.str() +
                     "'. Use RememberFunctionOrigin to store the origin before calling CreateIntfInlineCacheId");
    auto aotData = GetAotDataFromBuilder(callerOriginFile, aotBuilder_);
    uint64_t intfInlineCacheIndex = aotData.GetIntfInlineCacheIndex();
    int32_t slot = aotData.GetIntfInlineCacheSlotId(intfInlineCacheIndex);
    intfInlineCacheIndex++;
    aotData.SetIntfInlineCacheIndex(intfInlineCacheIndex);
    aotData.SetIsLLVMAotMode(true);
    return slot;
}

const char *LLVMArkInterface::GetFramePointerRegister() const
{
    switch (LLVMArchToArkArch(triple_.getArch())) {
        case Arch::AARCH64:
            return "x29";
        case Arch::X86_64:
            return "rbp";
        default:
            UNREACHABLE();
    }
}

void LLVMArkInterface::PutCalleeSavedRegistersMask(llvm::StringRef method, RegMasks masks)
{
    calleeSavedRegisters_.insert({method, masks});
}

LLVMArkInterface::RegMasks LLVMArkInterface::GetCalleeSavedRegistersMask(llvm::StringRef method)
{
    return calleeSavedRegisters_.lookup(method);
}

uintptr_t LLVMArkInterface::GetEntrypointsTablePointerTlsOffset() const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    return runtime_->GetEntrypointsTablePointerTlsOffset(arkArch);
}

uintptr_t LLVMArkInterface::GetEntrypointOffset(EntrypointId id) const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    using PandaEntrypointId = ark::compiler::RuntimeInterface::EntrypointId;
    return runtime_->GetEntrypointOffset(arkArch, static_cast<PandaEntrypointId>(id));
}

size_t LLVMArkInterface::GetTlsFrameKindOffset() const
{
    return runtime_->GetTlsFrameKindOffset(LLVMArchToArkArch(triple_.getArch()));
}

size_t LLVMArkInterface::GetTlsPreWrbEntrypointOffset() const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    return runtime_->GetTlsPreWrbEntrypointOffset(arkArch);
}

uint32_t LLVMArkInterface::GetClassOffset() const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    return runtime_->GetClassOffset(arkArch);
}

uint32_t LLVMArkInterface::GetManagedThreadPostWrbOneObjectOffset() const
{
    Arch arkArch = LLVMArchToArkArch(triple_.getArch());
    return cross_values::GetManagedThreadPostWrbOneObjectOffset(arkArch);
}

size_t LLVMArkInterface::GetStackOverflowCheckOffset() const
{
    return runtime_->GetStackOverflowCheckOffset();
}

std::string GetValueAsString(llvm::Value *value)
{
    std::string name;
    auto stream = llvm::raw_string_ostream(name);
    value->print(stream);
    return stream.str();
}

static bool MustLowerMemSet(const llvm::IntrinsicInst *inst, llvm::Triple::ArchType arch)
{
    // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
    ASSERT(inst->getIntrinsicID() == llvm::Intrinsic::memset ||
           inst->getIntrinsicID() == llvm::Intrinsic::memset_inline);

    auto sizeArg = inst->getArgOperand(2U);
    if (!llvm::isa<llvm::ConstantInt>(sizeArg)) {
        return true;
    }

    auto arraySize = llvm::cast<llvm::ConstantInt>(sizeArg)->getZExtValue();
    size_t smallSize;

    if (arch == llvm::Triple::ArchType::aarch64) {
        smallSize = MEMSET_SMALL_SIZE_FOR_AARCH64;
    } else {
        ASSERT(arch == llvm::Triple::ArchType::x86_64);
        bool sse42 = ark::compiler::g_options.IsCpuFeatureEnabled(ark::compiler::SSE42);
        smallSize = sse42 ? MEMSET_SMALL_SIZE_FOR_X86_64_SSE : MEMSET_SMALL_SIZE_FOR_X86_64_NO_SSE;
    }
    return arraySize > smallSize;
}

static bool MustLowerMemCpy(const llvm::IntrinsicInst *inst, llvm::Triple::ArchType arch)
{
    // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
    ASSERT(inst->getIntrinsicID() == llvm::Intrinsic::memcpy ||
           inst->getIntrinsicID() == llvm::Intrinsic::memcpy_inline);

    auto sizeArg = inst->getArgOperand(2U);
    if (!llvm::isa<llvm::ConstantInt>(sizeArg)) {
        return true;
    }

    auto arraySize = llvm::cast<llvm::ConstantInt>(sizeArg)->getZExtValue();
    if (arch == llvm::Triple::ArchType::x86_64) {
        bool sse42 = ark::compiler::g_options.IsCpuFeatureEnabled(ark::compiler::SSE42);
        if (sse42) {
            return arraySize > MEMCPY_BIG_SIZE_FOR_X86_64_SSE;
        }
        return arraySize > MEMCPY_BIG_SIZE_FOR_X86_64_NO_SSE;
    }

    ASSERT(arch == llvm::Triple::ArchType::aarch64);

    return arraySize > MEMCPY_BIG_SIZE_FOR_AARCH64;
}

static bool MustLowerMemMove(const llvm::IntrinsicInst *inst, llvm::Triple::ArchType arch)
{
    // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
    ASSERT(inst->getIntrinsicID() == llvm::Intrinsic::memmove);

    auto sizeArg = inst->getArgOperand(2U);
    if (!llvm::isa<llvm::ConstantInt>(sizeArg)) {
        return true;
    }

    auto arraySize = llvm::cast<llvm::ConstantInt>(sizeArg)->getZExtValue();

    size_t bigSize;
    size_t smallSize;
    int maxPopcount;
    if (arch == llvm::Triple::ArchType::aarch64) {
        smallSize = MEMMOVE_SMALL_SIZE_FOR_AARCH64;
        bigSize = MEMMOVE_BIG_SIZE_FOR_AARCH64;
        maxPopcount = 3U;
    } else {
        ASSERT(arch == llvm::Triple::ArchType::x86_64);
        if (ark::compiler::g_options.IsCpuFeatureEnabled(ark::compiler::SSE42)) {
            smallSize = MEMMOVE_SMALL_SIZE_FOR_X86_64_SSE;
            bigSize = MEMMOVE_BIG_SIZE_FOR_X86_64_SSE;
        } else {
            smallSize = MEMMOVE_SMALL_SIZE_FOR_X86_64_NO_SSE;
            bigSize = MEMMOVE_BIG_SIZE_FOR_X86_64_NO_SSE;
        }
        maxPopcount = 4U;
    }
    if (arraySize <= smallSize) {
        return false;
    }
    if (arraySize > bigSize) {
        return true;
    }
    return Popcount(arraySize) > maxPopcount;
}

llvm::Intrinsic::ID LLVMArkInterface::GetLLVMIntrinsicId(const llvm::Instruction *inst) const
{
    ASSERT(inst != nullptr);

    auto intrinsicInst = llvm::dyn_cast<llvm::IntrinsicInst>(inst);
    if (intrinsicInst == nullptr) {
        return llvm::Intrinsic::not_intrinsic;
    }

    auto llvmId = intrinsicInst->getIntrinsicID();
    // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
    if (llvmId == llvm::Intrinsic::memcpy && !MustLowerMemCpy(intrinsicInst, triple_.getArch())) {
        return llvm::Intrinsic::memcpy_inline;
    }
    // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
    if (llvmId == llvm::Intrinsic::memset && !MustLowerMemSet(intrinsicInst, triple_.getArch())) {
        return llvm::Intrinsic::memset_inline;
    }

    return llvm::Intrinsic::not_intrinsic;
}

[[maybe_unused]] static bool X86NoSSE(llvm::Triple::ArchType arch)
{
    return arch == llvm::Triple::ArchType::x86_64 &&
           !ark::compiler::g_options.IsCpuFeatureEnabled(ark::compiler::SSE42);
}

#include "get_intrinsic_id_llvm_ark_interface_gen.inl"

LLVMArkInterface::IntrinsicId LLVMArkInterface::GetIntrinsicId(const llvm::Instruction *inst) const
{
    ASSERT(inst != nullptr);
    auto opcode = inst->getOpcode();
    auto type = inst->getType();

    if (opcode == llvm::Instruction::FRem) {
        ASSERT(inst->getNumOperands() == 2U);
        ASSERT(type->isFloatTy() || type->isDoubleTy());
        auto id = type->isFloatTy() ? RuntimeInterface::IntrinsicId::LIB_CALL_FMODF
                                    : RuntimeInterface::IntrinsicId::LIB_CALL_FMOD;
        return static_cast<IntrinsicId>(id);
    }

    auto pluginResult = GetPluginIntrinsicId(inst);
    if (pluginResult != NO_INTRINSIC_ID_CONTINUE) {
        return pluginResult;
    }

    if (const auto *call = llvm::dyn_cast<llvm::CallInst>(inst)) {
        if (const llvm::Function *func = call->getCalledFunction()) {
            // Can be generated in instcombine for `pow` intrinsic
            if (func->getName() == "ldexp" || func->getName() == "ldexpf") {
                ASSERT(type->isDoubleTy() || type->isFloatTy());
                EVENT_PAOC("Lowering @ldexp");
                auto id = type->isDoubleTy() ? RuntimeInterface::IntrinsicId::LIB_CALL_LDEXP
                                             : RuntimeInterface::IntrinsicId::LIB_CALL_LDEXPF;
                return static_cast<IntrinsicId>(id);
            }
        }
    }

    if (llvm::isa<llvm::IntrinsicInst>(inst)) {
        return GetIntrinsicIdSwitch(llvm::cast<llvm::IntrinsicInst>(inst));
    }
    return NO_INTRINSIC_ID;
}

LLVMArkInterface::IntrinsicId LLVMArkInterface::GetIntrinsicIdSwitch(const llvm::IntrinsicInst *inst) const
{
    switch (inst->getIntrinsicID()) {
        // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
        case llvm::Intrinsic::memcpy:
        case llvm::Intrinsic::memcpy_inline:
        // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
        case llvm::Intrinsic::memmove:
        // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
        case llvm::Intrinsic::memset:
        case llvm::Intrinsic::memset_inline:
            return GetIntrinsicIdMemory(inst);
        case llvm::Intrinsic::minimum:
        case llvm::Intrinsic::maximum:
        case llvm::Intrinsic::sin:
        case llvm::Intrinsic::cos:
        case llvm::Intrinsic::pow:
        case llvm::Intrinsic::exp2:
            return GetIntrinsicIdMath(inst);
        case llvm::Intrinsic::ceil:
        case llvm::Intrinsic::floor:
        case llvm::Intrinsic::rint:
        case llvm::Intrinsic::round:
        case llvm::Intrinsic::lround:
        case llvm::Intrinsic::exp:
        case llvm::Intrinsic::log:
        case llvm::Intrinsic::log2:
        case llvm::Intrinsic::log10:
            UNREACHABLE();
        default:
            return NO_INTRINSIC_ID;
    }
    UNREACHABLE();
    return NO_INTRINSIC_ID;
}

LLVMArkInterface::IntrinsicId LLVMArkInterface::GetIntrinsicIdMemory(const llvm::IntrinsicInst *inst) const
{
    auto arch = triple_.getArch();
    auto llvmId = inst->getIntrinsicID();
    switch (llvmId) {
        // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
        case llvm::Intrinsic::memcpy:
        case llvm::Intrinsic::memcpy_inline:
            if (!ark::compiler::g_options.IsCompilerNonOptimizing() && !MustLowerMemCpy(inst, arch)) {
                // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
                ASSERT(llvmId != llvm::Intrinsic::memcpy);
                EVENT_PAOC("Skip lowering for @llvm.memcpy");
                return NO_INTRINSIC_ID;
            }
            EVENT_PAOC("Lowering @llvm.memcpy");
            return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY);
        // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
        case llvm::Intrinsic::memmove:
            if (!ark::compiler::g_options.IsCompilerNonOptimizing() && !MustLowerMemMove(inst, arch)) {
                EVENT_PAOC("Skip lowering for @llvm.memmove, size is: " + GetValueAsString(inst->getArgOperand(2U)));
                return NO_INTRINSIC_ID;
            }
            EVENT_PAOC("Lowering @llvm.memmove, size is: " + GetValueAsString(inst->getArgOperand(2U)));
            return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::LIB_CALL_MEM_MOVE);
        // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
        case llvm::Intrinsic::memset:
        case llvm::Intrinsic::memset_inline:
            if (!ark::compiler::g_options.IsCompilerNonOptimizing() && !MustLowerMemSet(inst, arch)) {
                // CC-OFFNXT(RiskyFunction, unsafe_function) false positive
                ASSERT(llvmId != llvm::Intrinsic::memset);
                EVENT_PAOC("Skip lowering for @llvm.memset");
                return NO_INTRINSIC_ID;
            }
            EVENT_PAOC("Lowering @llvm.memset");
            return static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::LIB_CALL_MEM_SET);
        default:
            UNREACHABLE();
            return NO_INTRINSIC_ID;
    }
    return NO_INTRINSIC_ID;
}

LLVMArkInterface::IntrinsicId LLVMArkInterface::GetIntrinsicIdMath(const llvm::IntrinsicInst *inst) const
{
    auto arch = triple_.getArch();
    auto type = inst->getType();
    auto llvmId = inst->getIntrinsicID();
    auto etype = !type->isVectorTy() ? type : llvm::cast<llvm::VectorType>(type)->getElementType();
    ASSERT(etype->isFloatTy() || etype->isDoubleTy());
    switch (llvmId) {
        case llvm::Intrinsic::minimum: {
            if (arch == llvm::Triple::ArchType::aarch64) {
                return NO_INTRINSIC_ID;
            }
            auto id = etype->isFloatTy() ? RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32
                                         : RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64;
            return static_cast<IntrinsicId>(id);
        }
        case llvm::Intrinsic::maximum: {
            if (arch == llvm::Triple::ArchType::aarch64) {
                return NO_INTRINSIC_ID;
            }
            auto id = etype->isFloatTy() ? RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32
                                         : RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64;
            return static_cast<IntrinsicId>(id);
        }
        case llvm::Intrinsic::sin: {
            auto id = etype->isFloatTy() ? RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SIN_F32
                                         : RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SIN_F64;
            return static_cast<IntrinsicId>(id);
        }
        case llvm::Intrinsic::cos: {
            auto id = etype->isFloatTy() ? RuntimeInterface::IntrinsicId::INTRINSIC_MATH_COS_F32
                                         : RuntimeInterface::IntrinsicId::INTRINSIC_MATH_COS_F64;
            return static_cast<IntrinsicId>(id);
        }
        case llvm::Intrinsic::pow: {
            auto id = etype->isFloatTy() ? RuntimeInterface::IntrinsicId::INTRINSIC_MATH_POW_F32
                                         : RuntimeInterface::IntrinsicId::INTRINSIC_MATH_POW_F64;
            return static_cast<IntrinsicId>(id);
        }
        case llvm::Intrinsic::exp2: {
            auto id = etype->isFloatTy() ? RuntimeInterface::IntrinsicId::LIB_CALL_EXP2F
                                         : RuntimeInterface::IntrinsicId::LIB_CALL_EXP2;
            return static_cast<IntrinsicId>(id);
        }
        default:
            UNREACHABLE();
            return NO_INTRINSIC_ID;
    }
    return NO_INTRINSIC_ID;
}

llvm::StringRef LLVMArkInterface::GetRuntimeFunctionName(LLVMArkInterface::RuntimeCallType callType,
                                                         LLVMArkInterface::IntrinsicId id)
{
    if (callType == LLVMArkInterface::RuntimeCallType::INTRINSIC) {
        return llvm::StringRef(GetIntrinsicRuntimeFunctionName(id));
    }
    // sanity check
    ASSERT(callType == LLVMArkInterface::RuntimeCallType::ENTRYPOINT);
    return llvm::StringRef(GetEntrypointRuntimeFunctionName(id));
}

llvm::FunctionType *LLVMArkInterface::GetRuntimeFunctionType(llvm::StringRef name) const
{
#ifndef NDEBUG
    ASSERT_PRINT(!compiled_, "Attempt to call GetRuntimeFunctionType after module is compiled");
#endif
    return runtimeFunctionTypes_.lookup(name);
}

llvm::FunctionType *LLVMArkInterface::GetOrCreateRuntimeFunctionType(llvm::LLVMContext &ctx, llvm::Module *module,
                                                                     LLVMArkInterface::RuntimeCallType callType,
                                                                     LLVMArkInterface::IntrinsicId id)
{
#ifndef NDEBUG
    ASSERT_PRINT(!compiled_, "Attempt to call GetOrCreateRuntimeFunctionType after module is compiled");
#endif
    auto rtFunctionName = GetRuntimeFunctionName(callType, id);
    auto rtFunctionTy = GetRuntimeFunctionType(rtFunctionName);
    if (rtFunctionTy != nullptr) {
        return rtFunctionTy;
    }

    if (callType == RuntimeCallType::INTRINSIC) {
        rtFunctionTy = GetIntrinsicDeclaration(ctx, static_cast<RuntimeInterface::IntrinsicId>(id));
    } else {
        // sanity check
        ASSERT(callType == RuntimeCallType::ENTRYPOINT);
        rtFunctionTy = GetEntrypointDeclaration(ctx, module, static_cast<RuntimeInterface::EntrypointId>(id));
    }

    ASSERT(rtFunctionTy != nullptr);
    runtimeFunctionTypes_.insert({rtFunctionName, rtFunctionTy});
    return rtFunctionTy;
}

#ifndef NDEBUG
void LLVMArkInterface::MarkAsCompiled()
{
    runtimeFunctionTypes_.clear();
    compiled_ = true;
}
#endif

void LLVMArkInterface::CreateRequiredIntrinsicFunctionTypes(llvm::LLVMContext &ctx)
{
    constexpr std::array REQUIRED_INTRINSIC_IDS = {
        RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32,
        RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32,
        RuntimeInterface::IntrinsicId::INTRINSIC_MATH_POW_F64, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_POW_F32,
        RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SIN_F64, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SIN_F32,
        RuntimeInterface::IntrinsicId::INTRINSIC_MATH_COS_F64, RuntimeInterface::IntrinsicId::INTRINSIC_MATH_COS_F32,
        RuntimeInterface::IntrinsicId::LIB_CALL_FMODF,         RuntimeInterface::IntrinsicId::LIB_CALL_FMOD,
        RuntimeInterface::IntrinsicId::LIB_CALL_LDEXP,         RuntimeInterface::IntrinsicId::LIB_CALL_LDEXPF,
        RuntimeInterface::IntrinsicId::LIB_CALL_EXP2,          RuntimeInterface::IntrinsicId::LIB_CALL_EXP2F,
        RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY,      RuntimeInterface::IntrinsicId::LIB_CALL_MEM_MOVE,
        RuntimeInterface::IntrinsicId::LIB_CALL_MEM_SET};
    for (auto id : REQUIRED_INTRINSIC_IDS) {
        auto rtFunctionName =
            GetRuntimeFunctionName(RuntimeCallType::INTRINSIC, static_cast<LLVMArkInterface::IntrinsicId>(id));
        auto rtFunctionTy = GetIntrinsicDeclaration(ctx, id);
        ASSERT(rtFunctionTy != nullptr);
        runtimeFunctionTypes_.insert({rtFunctionName, rtFunctionTy});
    }
}

void LLVMArkInterface::RememberFunctionOrigin(const llvm::Function *function, MethodPtr methodPtr)
{
    ASSERT(function != nullptr);
    ASSERT(methodPtr != nullptr);
    auto funcName = function->getName();
    auto file = static_cast<panda_file::File *>(runtime_->GetBinaryFileForMethod(methodPtr));
    [[maybe_unused]] auto insertionResultFunctionOrigins = functionOrigins_.insert({funcName, file});
    ASSERT(insertionResultFunctionOrigins.second);
    LLVM_LOG(DEBUG, INFRA) << funcName.data() << " defined in " << runtime_->GetFileName(methodPtr);
    [[maybe_unused]] auto insertionResultMethodPtrs = methodPtrs_.insert({funcName, methodPtr});
    ASSERT(insertionResultMethodPtrs.second);
}

LLVMArkInterface::MethodId LLVMArkInterface::GetMethodId(const llvm::Function *caller,
                                                         const llvm::Function *callee) const
{
    ASSERT(callee != nullptr);
    ASSERT(caller != nullptr);
    auto callerName = caller->getName();
    auto callerOriginFile = functionOrigins_.lookup(callerName);
    ASSERT_PRINT(callerOriginFile != nullptr,
                 std::string("No origin for function = '") + callerName.str() +
                     "'. Use RememberFunctionOrigin to store the origin before calling GetMethodId");
    auto calleeName = callee->getName();
    auto callerOriginFileIterator = methodIds_.find(callerOriginFile);
    ASSERT_PRINT(callerOriginFileIterator != methodIds_.end(), "No method ids were declared in callerOriginFile");
    auto methodIdIterator = callerOriginFileIterator->second.find(calleeName);
    ASSERT_PRINT(methodIdIterator != callerOriginFileIterator->second.end(),
                 std::string(calleeName) + " was not declared in callerOriginFile");

    return methodIdIterator->second;
}

void LLVMArkInterface::RememberFunctionCall(const llvm::Function *caller, const llvm::Function *callee,
                                            MethodId methodId)
{
    ASSERT(callee != nullptr);
    ASSERT(caller != nullptr);
    auto callerName = caller->getName();
    auto pandaFile = functionOrigins_.lookup(callerName);
    ASSERT_PRINT(pandaFile != nullptr,
                 std::string("No origin for function = '") + callerName.str() +
                     "'. Use RememberFunctionOrigin to store the origin before calling RememberFunctionCall");
    // Assuming it is ok to declare extern function twice
    auto &pandaFileMap = methodIds_[pandaFile];
    pandaFileMap.insert({callee->getName(), methodId});
}

bool ark::llvmbackend::LLVMArkInterface::IsRememberedCall(const llvm::Function *caller,
                                                          const llvm::Function *callee) const
{
    ASSERT(caller != nullptr);
    ASSERT(callee != nullptr);
    auto callerName = caller->getName();
    auto callerOriginFile = functionOrigins_.find(callerName);
    ASSERT_PRINT(callerOriginFile != functionOrigins_.end(),
                 callerName.str() + " has no origin panda file. Use RememberFunctionOrigin for " + callerName.str() +
                     " before calling IsRememberedCall");
    auto callerOriginFileIterator = methodIds_.find(callerOriginFile->second);
    return callerOriginFileIterator != methodIds_.end() &&
           callerOriginFileIterator->second.find(callee->getName()) != callerOriginFileIterator->second.end();
}

int32_t ark::llvmbackend::LLVMArkInterface::GetObjectClassOffset() const
{
    auto arch = LLVMArchToArkArch(triple_.getArch());
    return runtime_->GetObjClassOffset(arch);
}

bool ark::llvmbackend::LLVMArkInterface::IsExternal(llvm::CallInst *call)
{
    auto caller = methodPtrs_.find(call->getFunction()->getName());
    ASSERT(caller != methodPtrs_.end());
    auto callee = methodPtrs_.find(call->getCalledFunction()->getName());
    ASSERT(callee != methodPtrs_.end());
    return runtime_->IsMethodExternal(caller->second, callee->second);
}

LLVMArkInterface::MethodPtr ark::llvmbackend::LLVMArkInterface::ResolveVirtual(uint32_t classId, llvm::CallInst *call)
{
    auto caller = methodPtrs_.find(call->getFunction()->getName());
    ASSERT(caller != methodPtrs_.end());
    auto klass = runtime_->GetClass(caller->second, classId);
    if (klass == nullptr) {
        return nullptr;
    }
    auto callee = methodPtrs_.find(call->getCalledFunction()->getName());
    ASSERT(callee != methodPtrs_.end());
    auto calleeClassId = runtime_->GetClassIdForMethod(callee->second);
    auto calleeClass = runtime_->GetClass(callee->second, calleeClassId);
    if (!runtime_->IsAssignableFrom(calleeClass, klass)) {
        return nullptr;
    }
    auto resolved = runtime_->ResolveVirtualMethod(klass, callee->second);
    return resolved;
}

int32_t LLVMArkInterface::GetPltSlotId(const llvm::Function *caller, const llvm::Function *callee) const
{
    ASSERT(caller != nullptr);
    ASSERT(callee != nullptr);
    auto callerName = caller->getName();
    llvm::sys::ScopedLock scopedLock {*lock_};
    auto callerOriginFile = functionOrigins_.lookup(callerName);
    ASSERT_PRINT(callerOriginFile != nullptr,
                 std::string("No origin for function = '") + callerName.str() +
                     "'. Use RememberFunctionOrigin to store the origin before calling GetPltSlotId");
    auto aotData = GetAotDataFromBuilder(callerOriginFile, aotBuilder_);
    aotData.SetIsLLVMAotMode(true);
    return aotData.GetPltSlotId(GetMethodId(caller, callee), 0);
}

int32_t LLVMArkInterface::GetClassIndexInAotGot(ark::compiler::AotData *aotData, uint32_t klassId, bool initialized)
{
    ASSERT(aotData != nullptr);
    llvm::sys::ScopedLock scopedLock {*lock_};
    auto index = aotData->GetClassSlotId(klassId, 0);
    if (initialized) {
        return index - 1;
    }
    return index;
}

int32_t LLVMArkInterface::GetStringSlotId(AotData *aotData, uint32_t typeId)
{
    ASSERT(aotData != nullptr);
    llvm::sys::ScopedLock scopedLock {*lock_};
    return aotData->GetStringSlotId(typeId, 0);
}

LLVMArkInterface::RuntimeCallee LLVMArkInterface::GetEntrypointCallee(EntrypointId id) const
{
    using PandaEntrypointId = ark::compiler::RuntimeInterface::EntrypointId;
    auto eid = static_cast<PandaEntrypointId>(id);
    auto functionName = GetEntrypointInternalName(eid);
    auto functionProto = GetRuntimeFunctionType(functionName);
    ASSERT(functionProto != nullptr);
    return {functionProto, functionName};
}

BridgeType LLVMArkInterface::GetBridgeType(EntrypointId id) const
{
    return GetBridgeTypeInternal(static_cast<RuntimeInterface::EntrypointId>(id));
}

void LLVMArkInterface::PutVirtualFunction(LLVMArkInterface::MethodPtr methodPtr, llvm::Function *function)
{
    ASSERT(function != nullptr);
    ASSERT(methodPtr != nullptr);

    methodPtrs_.insert({function->getName(), methodPtr});
}

uint32_t LLVMArkInterface::GetVTableOffset(llvm::Function *caller, uint32_t methodId) const
{
    auto arkCaller = methodPtrs_.find(caller->getName());
    auto callee = runtime_->GetMethodById(arkCaller->second, methodId);

    auto arch = LLVMArchToArkArch(triple_.getArch());
    auto vtableIndex = runtime_->GetVTableIndex(callee);
    uint32_t offset = runtime_->GetVTableOffset(arch) + (vtableIndex << 3U);
    return offset;
}

bool LLVMArkInterface::IsInterfaceMethod(llvm::CallInst *call)
{
    auto methodId = utils::GetMethodIdFromAttr(call);
    auto caller = methodPtrs_.find(call->getFunction()->getName());
    auto callee = runtime_->GetMethodById(caller->second, methodId);

    return runtime_->IsInterfaceMethod(callee);
}

const char *LLVMArkInterface::GetIntrinsicRuntimeFunctionName(LLVMArkInterface::IntrinsicId id) const
{
    ASSERT(id >= 0 && id < static_cast<IntrinsicId>(RuntimeInterface::IntrinsicId::COUNT));
    return GetIntrinsicInternalName(static_cast<RuntimeInterface::IntrinsicId>(id));
}

const char *LLVMArkInterface::GetEntrypointRuntimeFunctionName(LLVMArkInterface::EntrypointId id) const
{
    ASSERT(id >= 0 && id < static_cast<EntrypointId>(RuntimeInterface::EntrypointId::COUNT));
    return GetEntrypointInternalName(static_cast<RuntimeInterface::EntrypointId>(id));
}

std::string LLVMArkInterface::GetUniqMethodName(LLVMArkInterface::MethodPtr methodPtr) const
{
    ASSERT(methodPtr != nullptr);

    if (IsIrtocMode()) {
        return runtime_->GetMethodName(methodPtr);
    }
#ifndef NDEBUG
    auto uniqName = std::string(runtime_->GetMethodFullName(methodPtr, true));
    uniqName.append("_id_");
    uniqName.append(std::to_string(runtime_->GetUniqMethodId(methodPtr)));
#else
    std::stringstream ssUniqName;
    ssUniqName << "f_" << std::hex << methodPtr;
    auto uniqName = ssUniqName.str();
#endif
    return uniqName;
}

std::string LLVMArkInterface::GetUniqMethodName(const Method *method) const
{
    ASSERT(method != nullptr);

    auto casted = const_cast<Method *>(method);
    return GetUniqMethodName(static_cast<MethodPtr>(casted));
}

std::string LLVMArkInterface::GetUniqueBasicBlockName(const std::string &bbName, const std::string &uniqueSuffix)
{
    std::stringstream uniqueName;
    const std::string nameDelimiter = "..";
    auto first = bbName.find(nameDelimiter);
    if (first == std::string::npos) {
        uniqueName << bbName << "_" << uniqueSuffix;
        return uniqueName.str();
    }

    auto second = bbName.rfind(nameDelimiter);
    ASSERT(second != std::string::npos);

    uniqueName << bbName.substr(0, first + 2U) << uniqueSuffix << bbName.substr(second, bbName.size());

    return uniqueName.str();
}

uint64_t LLVMArkInterface::GetMethodStackSize(LLVMArkInterface::MethodPtr method) const
{
    std::string name = GetUniqMethodName(method);
    ASSERT(llvmStackSizes_.find(name) != llvmStackSizes_.end());
    return llvmStackSizes_.lookup(name);
}

void LLVMArkInterface::PutMethodStackSize(const llvm::Function *method, uint64_t size)
{
    std::string name = method->getName().str();
    ASSERT_PRINT(llvmStackSizes_.find(name) == llvmStackSizes_.end(), "Double inserted stack size for '" + name + "'");
    llvmStackSizes_[name] = size;
}

uint32_t LLVMArkInterface::GetVirtualRegistersCount(LLVMArkInterface::MethodPtr method) const
{
    // Registers used by user, arguments and accumulator
    return runtime_->GetMethodRegistersCount(method) + runtime_->GetMethodArgumentsCount(method) + 1;
}

uint32_t LLVMArkInterface::GetCFrameHasFloatRegsFlagMask() const
{
    return 1U << ark::CFrameLayout::HasFloatRegsFlag::START_BIT;
}

bool LLVMArkInterface::IsIrtocMode() const
{
    return aotBuilder_ == nullptr;
}

void LLVMArkInterface::AppendIrtocReturnHandler(llvm::StringRef returnHandler)
{
    irtocReturnHandlers_.push_back(returnHandler);
}

bool LLVMArkInterface::IsIrtocReturnHandler(const llvm::Function &function) const
{
    return std::find(irtocReturnHandlers_.cbegin(), irtocReturnHandlers_.cend(), function.getName()) !=
           irtocReturnHandlers_.cend();
}

}  // namespace ark::llvmbackend
