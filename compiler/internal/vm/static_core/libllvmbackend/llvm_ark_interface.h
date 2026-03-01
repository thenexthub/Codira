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

#ifndef LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H
#define LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H

#include <map>
#include <unordered_map>

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/Triple.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/ValueMap.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Support/AtomicOrdering.h>
#include <llvm/Support/Mutex.h>

namespace llvm {
class Function;
class FunctionType;
class Instruction;
class Module;
}  // namespace llvm

namespace ark {
class Method;
}  // namespace ark

namespace ark::compiler {
class AotBuilder;
class AotData;
class RuntimeInterface;
class Graph;
}  // namespace ark::compiler

namespace ark::panda_file {
class File;
}  // namespace ark::panda_file

namespace ark::llvmbackend {

enum class BridgeType {
    NONE,        //
    ODD_SAVED,   //
    ODD_SAVED1,  //
    ODD_SAVED2,  //
    ODD_SAVED3,  //
    ODD_SAVED4,  //
    ENTRYPOINT,  //
    SLOW_PATH,   //
};

class LLVMArkInterface {
public:
    using MethodPtr = void *;
    using ClassPtr = void *;
    using MethodId = uint32_t;
    using IntrinsicId = int;
    using EntrypointId = int;
    using RuntimeCallee = std::pair<llvm::FunctionType *, llvm::StringRef>;
    using RegMasks = std::tuple<uint32_t, uint32_t>;
    enum class RuntimeCallType { INTRINSIC, ENTRYPOINT };

    explicit LLVMArkInterface(ark::compiler::RuntimeInterface *runtime, llvm::Triple triple,
                              ark::compiler::AotBuilder *aotBuilder, llvm::sys::Mutex *lock);

    RuntimeCallee GetEntrypointCallee(EntrypointId id) const;

    void PutCalleeSavedRegistersMask(llvm::StringRef method, RegMasks masks);

    RegMasks GetCalleeSavedRegistersMask(llvm::StringRef method);

    intptr_t GetCompiledEntryPointOffset() const;

    const char *GetThreadRegister() const;
    const char *GetFramePointerRegister() const;

    uintptr_t GetEntrypointsTablePointerTlsOffset() const;
    uintptr_t GetEntrypointOffset(EntrypointId id) const;
    size_t GetTlsFrameKindOffset() const;
    size_t GetTlsPreWrbEntrypointOffset() const;

    void PutVirtualFunction(MethodPtr methodPtr, llvm::Function *function);

    IntrinsicId GetIntrinsicId(const llvm::Instruction *inst) const;
    llvm::Intrinsic::ID GetLLVMIntrinsicId(const llvm::Instruction *inst) const;

    const char *GetIntrinsicRuntimeFunctionName(LLVMArkInterface::IntrinsicId id) const;
    const char *GetEntrypointRuntimeFunctionName(LLVMArkInterface::EntrypointId id) const;
    BridgeType GetBridgeType(EntrypointId id) const;

    llvm::StringRef GetRuntimeFunctionName(LLVMArkInterface::RuntimeCallType callType, IntrinsicId id);
    llvm::FunctionType *GetRuntimeFunctionType(llvm::StringRef name) const;
    llvm::FunctionType *GetOrCreateRuntimeFunctionType(llvm::LLVMContext &ctx, llvm::Module *module,
                                                       LLVMArkInterface::RuntimeCallType callType, IntrinsicId id);
#ifndef NDEBUG
    void MarkAsCompiled();
#endif

    void CreateRequiredIntrinsicFunctionTypes(llvm::LLVMContext &ctx);

    bool IsRememberedCall(const llvm::Function *caller, const llvm::Function *callee) const;
    void RememberFunctionOrigin(const llvm::Function *function, MethodPtr methodPtr);

    std::string GetUniqMethodName(MethodPtr methodPtr) const;
    std::string GetUniqMethodName(const Method *method) const;
    static std::string GetUniqueBasicBlockName(const std::string &bbName, const std::string &uniqueSuffix);

    MethodId GetMethodId(const llvm::Function *caller, const llvm::Function *callee) const;

    uint32_t GetClassOffset() const;
    uint32_t GetManagedThreadPostWrbOneObjectOffset() const;
    size_t GetStackOverflowCheckOffset() const;
    uint32_t GetVTableOffset(llvm::Function *caller, uint32_t methodId) const;

    void RememberFunctionCall(const llvm::Function *caller, const llvm::Function *callee, MethodId methodId);

    int32_t GetPltSlotId(const llvm::Function *caller, const llvm::Function *callee) const;
    int32_t GetObjectClassOffset() const;
    int32_t GetClassIndexInAotGot(ark::compiler::AotData *aotData, uint32_t klassId, bool initialized);
    int32_t GetStringSlotId(ark::compiler::AotData *aotData, uint32_t typeId);
    uint64_t GetMethodStackSize(MethodPtr method) const;
    void PutMethodStackSize(const llvm::Function *method, size_t size);
    uint32_t GetVirtualRegistersCount(MethodPtr method) const;
    uint32_t GetCFrameHasFloatRegsFlagMask() const;

    bool IsIrtocMode() const;
    bool IsArm64() const
    {
        return triple_.getArch() == llvm::Triple::ArchType::aarch64;
    }

    void AppendIrtocReturnHandler(llvm::StringRef returnHandler);

    bool IsIrtocReturnHandler(const llvm::Function &function) const;

    int32_t CreateIntfInlineCacheSlotId(const llvm::Function *caller) const;

    MethodPtr ResolveVirtual(uint32_t classId, llvm::CallInst *call);
    bool IsInterfaceMethod(llvm::CallInst *call);
    bool IsExternal(llvm::CallInst *call);

public:
    static constexpr auto NO_INTRINSIC_ID = static_cast<IntrinsicId>(-1);
    static constexpr auto GC_ADDR_SPACE = 271;
    static constexpr auto VOLATILE_ORDER = llvm::AtomicOrdering::SequentiallyConsistent;
    static constexpr auto NOT_ATOMIC_ORDER = llvm::AtomicOrdering::NotAtomic;
    static constexpr std::string_view GC_SAFEPOINT_POLL_NAME = "gc.safepoint_poll";
    static constexpr std::string_view GC_STRATEGY = "ark";
    static constexpr std::string_view FUNCTION_MD_CLASS_ID = "class_id";
    static constexpr std::string_view FUNCTION_MD_INLINE_MODULE = "inline_module";
    static constexpr std::string_view PATCH_STACK_ADJUSTMENT_COMMENT = " ${:comment} patch-stack-adjustment";
    static constexpr std::string_view AARCH64_SDIV_INST = "aarch64_sdiv";
    static constexpr std::string_view SOURCE_LANG_ATTR = "source-language";

    ark::compiler::RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    bool DeoptsEnabled();

    // Convert given dwarfReg id into Ark register id
    // For dwarf numbering see https://www.ucw.cz/~hubicka/papers/abi/node14.html
    // For Ark see static_core/compiler/optimizer/code_generator/target_info.h
    static constexpr size_t X86RegNumberConvert(size_t dwarfReg)
    {
        constexpr size_t RAX = 0U;   // dwarf 0
        constexpr size_t RDX = 2U;   // dwarf 1
        constexpr size_t RCX = 1U;   // dwarf 2
        constexpr size_t RBX = 11U;  // dwarf 3
        constexpr size_t RSI = 6U;   // dwarf 4
        constexpr size_t RDI = 7U;   // dwarf 5
        constexpr size_t RBP = 9U;   // dwarf 6
        constexpr size_t RSP = 10U;  // dwarf 7
        constexpr size_t R8 = 8U;    // dwarf 8
        constexpr size_t R9 = 5U;    // dwarf 9
        constexpr size_t R10 = 4U;   // dwarf 10
        constexpr size_t R11 = 3U;   // dwarf 11
        constexpr size_t R12 = 12U;  // dwarf 12

        constexpr std::array TO_ARK = {RAX, RDX, RCX, RBX, RSI, RDI, RBP, RSP, R8, R9, R10, R11};
        return dwarfReg >= R12 ? dwarfReg : TO_ARK[dwarfReg];
    }

    // Convert given Ark register id to dwarf register id
    static constexpr size_t ToDwarfRegNumX86(size_t arkReg)
    {
        constexpr size_t RAX = 0U;   // Ark 0
        constexpr size_t RCX = 2U;   // Ark 1
        constexpr size_t RDX = 1U;   // Ark 2
        constexpr size_t R11 = 11U;  // Ark 3
        constexpr size_t R10 = 10U;  // Ark 4
        constexpr size_t R9 = 9U;    // Ark 5
        constexpr size_t RSI = 4U;   // Ark 6
        constexpr size_t RDI = 5U;   // Ark 7
        constexpr size_t R8 = 8U;    // Ark 8
        constexpr size_t RBP = 6U;   // Ark 9
        constexpr size_t RSP = 7U;   // Ark 10
        constexpr size_t RBX = 3U;   // Ark 11
        constexpr size_t R12 = 12U;  // Ark 12

        constexpr std::array TO_DWARF = {RAX, RCX, RDX, R11, R10, R9, RSI, RDI, R8, RBP, RSP, RBX};
        return arkReg >= R12 ? arkReg : TO_DWARF[arkReg];
    }

private:
    IntrinsicId GetPluginIntrinsicId(const llvm::Instruction *instruction) const;
    IntrinsicId GetIntrinsicIdSwitch(const llvm::IntrinsicInst *inst) const;
    IntrinsicId GetIntrinsicIdMemory(const llvm::IntrinsicInst *inst) const;
    IntrinsicId GetIntrinsicIdMath(const llvm::IntrinsicInst *inst) const;
#include "get_intrinsic_id_llvm_ark_interface_gen.h.inl"
private:
    static constexpr auto NO_INTRINSIC_ID_CONTINUE = static_cast<IntrinsicId>(-2);

    ark::compiler::RuntimeInterface *runtime_;
    llvm::Triple triple_;
    llvm::StringMap<llvm::FunctionType *> runtimeFunctionTypes_;
    llvm::StringMap<const panda_file::File *> functionOrigins_;
    llvm::DenseMap<const panda_file::File *, llvm::StringMap<MethodId>> methodIds_;
    llvm::StringMap<MethodPtr> methodPtrs_;
    llvm::StringMap<uint64_t> llvmStackSizes_;
    llvm::StringMap<RegMasks> calleeSavedRegisters_;
    ark::compiler::AotBuilder *aotBuilder_;
    std::vector<llvm::StringRef> irtocReturnHandlers_;
    mutable llvm::sys::Mutex *lock_;
#ifndef NDEBUG
    bool compiled_ = false;
#endif
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_ARK_INTERFACE_H
