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

/**
 * @file
 *
 * This file defines classes used for codenative backend to generate code related to the CFFI feature.
 */

#ifndef CODIRA_CODEGEN_CODENATIVECGCFFI_H
#define CODIRA_CODEGEN_CODENATIVECGCFFI_H

#include "CGContext.h"
#include "CODENative/CGCFFI.h"
#include "IRBuilder.h"
#include "Codira/Utils/SafePointer.h"

namespace Codira {
namespace CodeGen {
// Maximum number of a small struct type can be splited.
const unsigned MAX_SPLIT_NUM = 2;

class ABIArgInfo {
public:
    virtual ~ABIArgInfo() = default;

    /**
     * Pass or return the type indirectly via a hidden pointer.
     */
    static ABIArgInfo GetIndirect()
    {
        return {};
    }

    /**
     * Pass or return the type directly using the normal converted LLVM IR type.
     */
    static ABIArgInfo GetDirect(llvm::Type* ty)
    {
        auto ai = ABIArgInfo();
        ai.types.emplace_back(ty);
        return ai;
    }

    /**
     * Only valid for aggregate types.
     * The structure should be expanded into consecutive type for its constituent fields when as parameter type.
     * Or be wrapped in a literal struct type when as return type.
     */
    static ABIArgInfo GetExpand(llvm::Type* ty1, llvm::Type* ty2)
    {
        auto ai = ABIArgInfo();
        ai.types.emplace_back(ty1);
        ai.types.emplace_back(ty2);
        return ai;
    }

    bool IsDirect() const
    {
        return types.size() == 1;
    }

    bool IsIndirect() const
    {
        return types.empty();
    }

    bool IsExpand() const
    {
        return types.size() == MAX_SPLIT_NUM;
    }

    size_t ArgNums() const
    {
        return types.size();
    }

    llvm::Type* operator[](const size_t idx) const
    {
        CODEC_ASSERT(idx < types.size());
        return types[idx];
    }

private:
    ABIArgInfo() = default;

    llvm::SmallVector<llvm::Type*, MAX_SPLIT_NUM> types;
};

enum class ProcessKind : uint8_t { NO_PROCESS, SKIP, DIRECT, INDIRECT, EXPAND };

enum class WordClass : uint8_t {
    NO_CLASS,    // Initial value used by classification algorithm.
    INTEGER,     // Contains Bool, Int*, UInt*, CPointer, CString in Codira.
    SSE,         // Contains Float*.
    SSEUP,       // Useless in Codira's CType currently.
    X87,         // Useless in Codira's CType currently.
    X87UP,       // Useless in Codira's CType currently.
    COMPLEX_X87, // Useless in Codira's CType currently.
    MEMORY       // Other. Types that will be passed and returned in memory via the stack.
};

// Maximum number of fields that a small struct type can contain.
const unsigned MAX_FIELD_NUM = 8;

class LinuxCODENativeCGCFFI : public CGCFFI {
public:
    explicit LinuxCODENativeCGCFFI(CGModule& cgMod) : ctx(cgMod.GetCGContext()), cgMod(cgMod)
    {
    }

    bool ProcessInvocation(
        const CHIR::FuncType& chirFuncTy, std::vector<CGValue*>& args, IRBuilder2& builder) override;
    bool NeedProcessParam(const CHIR::FuncType& chirFuncTy, size_t idx) override
    {
        auto found = cfuncMap.find(&chirFuncTy);
        if (found == cfuncMap.end()) {
            return false;
        }
        auto paramTys = chirFuncTy.GetParamTypes();
        CODEC_ASSERT(idx < paramTys.size() && paramTys[idx]);
        if (paramTys[idx]->IsVArray()) {
            return true;
        }
        auto& kinds = found->second.second;
        return idx < kinds.size() && (kinds[idx] == ProcessKind::DIRECT || kinds[idx] == ProcessKind::EXPAND);
    }

protected:
    /**
     * Get the codegen return type.
     * If it needs to be passed through parameter (sret), it will be emplace in @p params and return a void type.
     * If it is a incomplete struct type, this method will return nullptr
     * and related cfunc type will be a literal struct pointer type.
     * Otherwise, the actual return type will be returned.
     */
    llvm::Type* GetReturnType(CHIR::Type& chirTy, std::vector<llvm::Type*>& params);
    bool IsUnsizedStructTy(const CHIR::Type& chirTy) const
    {
        return chirTy.IsStruct() && !GetLLVMType(chirTy)->isSized();
    }
    llvm::Type* GetLLVMType(const CHIR::Type& chirTy) const
    {
        auto cgType = CGType::GetOrCreate(cgMod, &chirTy);
        CODEC_NULLPTR_CHECK(cgType);
        return cgType->GetLLVMType();
    }
    virtual void ProcessInvocationArg(CHIR::StructType& chirParamTy, ProcessKind kind, size_t& argIdx,
        std::vector<CGValue*>& args, IRBuilder2& builder) = 0;
    virtual llvm::Type* GetStructReturnType(CHIR::StructType& chirTy, std::vector<llvm::Type*>& params) = 0;

    std::unordered_map<Ptr<CHIR::Type>, ABIArgInfo> typeMap;
    std::unordered_map<Ptr<const CHIR::FuncType>, std::pair<llvm::FunctionType*, std::vector<ProcessKind>>> cfuncMap;
    CGContext& ctx;
    CGModule& cgMod;
};

class LinuxAmd64CODENativeCGCFFI : public LinuxCODENativeCGCFFI {
public:
    using FieldOffsets = llvm::SmallVector<size_t, MAX_FIELD_NUM>;

    explicit LinuxAmd64CODENativeCGCFFI(CGModule& cgMod) : LinuxCODENativeCGCFFI(cgMod)
    {
    }

    llvm::FunctionType* GetCFuncType(const CHIR::FuncType& chirFuncTy) override;
    void AddFunctionAttr(const CHIR::FuncType& chirFuncTy, llvm::Function& llvmFunc) override;
    llvm::Value* ProcessRetValue(const llvm::Type& retType, llvm::Value& val, IRBuilder2& builder) override;
    void ProcessParam(CHIR::Type& chirParamTy, LLVMFuncArgIt& arg, llvm::Value* place, IRBuilder2& builder) override;

protected:
    void ProcessInvocationArg(CHIR::StructType& chirParamTy, ProcessKind kind, size_t& argIdx,
        std::vector<CGValue*>& args, IRBuilder2& builder) override;
    llvm::Type* GetStructReturnType(CHIR::StructType& chirTy, std::vector<llvm::Type*>& params) override;

private:
    ProcessKind GetParamType(CHIR::Type& chirTy, int8_t& intNum, int8_t& sseNum, std::vector<llvm::Type*>& params);
    llvm::Type* GetParamType(llvm::Type* ty, const ABIArgInfo& info, int8_t& intNum, int8_t& sseNum) const;
    ABIArgInfo GetMappingArgInfo(CHIR::StructType& chirTy, bool isArg);
    llvm::Type* GetLowPartType(llvm::Type* type, std::vector<WordClass>& classes, bool isArg) const;
    std::vector<WordClass> ClassifyType(llvm::Type& type) const;
    void ClassifyType(llvm::Type& type, const std::vector<WordClass>::iterator begin, size_t offset) const;
    void ClassifyStructType(
        llvm::ArrayRef<llvm::Type*> fields, const std::vector<WordClass>::iterator begin, size_t offset) const;
    void FixupClassification(const llvm::Type& type, std::vector<WordClass>& wordClasses) const;
    llvm::Type* GetIntegerTypeWithOffset(
        llvm::Type& curType, size_t offset, llvm::Type& srcType, size_t srcOffset) const;
    llvm::Type* GetSseTypeWithOffset(llvm::Type& curType, size_t offset, llvm::Type& srcType, size_t srcOffset) const;
    bool ContainsUselessBits(llvm::Type& type, size_t start, size_t end) const;
    FieldOffsets CalcFieldOffsets(const llvm::StructType& type) const;
    unsigned GetFieldContainsOffset(FieldOffsets fieldOffsets, size_t offset) const;
    bool ContainsFloatAtOffset(llvm::Type& type, size_t offset) const;
};

class LinuxAarch64CODENativeCGCFFI : public LinuxCODENativeCGCFFI {
public:
    explicit LinuxAarch64CODENativeCGCFFI(CGModule& cgMod) : LinuxCODENativeCGCFFI(cgMod)
    {
    }

    llvm::FunctionType* GetCFuncType(const CHIR::FuncType& chirFuncTy) override;
    void AddFunctionAttr(const CHIR::FuncType& chirFuncTy, llvm::Function& llvmFunc) override;
    void ProcessParam(CHIR::Type& chirParamTy, LLVMFuncArgIt& arg, llvm::Value* place, IRBuilder2& builder) override;
    llvm::Value* ProcessRetValue(const llvm::Type& retType, llvm::Value& val, IRBuilder2& builder) override;

protected:
    void ProcessInvocationArg(CHIR::StructType& chirParamTy, ProcessKind kind, size_t& argIdx,
        std::vector<CGValue*>& args, IRBuilder2& builder) override;
    llvm::Type* GetStructReturnType(CHIR::StructType& chirTy, std::vector<llvm::Type*>& params) override;
    bool IsHomogeneousAggregate(llvm::Type& type, llvm::Type*& base, size_t& members);

private:
    ProcessKind GetParamType(CHIR::Type& chirTy, std::vector<llvm::Type*>& params);
    ABIArgInfo GetMappingArgInfo(CHIR::StructType& chirTy, bool isArg);
};

#ifdef __APPLE__
class MacAArch64CODENativeCGCFFI : public LinuxAarch64CODENativeCGCFFI {
public:
    explicit MacAArch64CODENativeCGCFFI(CGModule& cgMod) : LinuxAarch64CODENativeCGCFFI(cgMod)
    {
    }

    void AddFunctionAttr(const CHIR::FuncType& chirFuncTy, llvm::Function& llvmFunc) override;
    llvm::FunctionType* GetCFuncType(const CHIR::FuncType& chirFuncTy) override;
    void ProcessParam(CHIR::Type& chirParamTy, LLVMFuncArgIt& arg, llvm::Value* place, IRBuilder2& builder) override;

protected:
    void ProcessInvocationArg(CHIR::StructType& chirParamTy, ProcessKind kind, size_t& argIdx,
        std::vector<CGValue*>& args, IRBuilder2& builder) override;
    llvm::Type* GetStructReturnType(CHIR::StructType& chirTy, std::vector<llvm::Type*>& params) override;

    std::unordered_map<Ptr<CHIR::Type>, ABIArgInfo> paramTypeMap;

private:
    ProcessKind GetParamType(CHIR::Type& chirTy, std::vector<llvm::Type*>& params);
    ABIArgInfo GetMappingArgInfo(CHIR::StructType& chirTy, bool isArg);
};
#endif

class WindowsAmd64CODENativeCGCFFI : public LinuxCODENativeCGCFFI {
public:
    explicit WindowsAmd64CODENativeCGCFFI(CGModule& cgMod) : LinuxCODENativeCGCFFI(cgMod)
    {
    }

    llvm::FunctionType* GetCFuncType(const CHIR::FuncType& chirFuncTy) override;
    void ProcessParam(CHIR::Type& chirParamTy, LLVMFuncArgIt& arg, llvm::Value* place, IRBuilder2& builder) override;
    llvm::Value* ProcessRetValue(const llvm::Type& retType, llvm::Value& val, IRBuilder2& builder) override;
    void AddFunctionAttr(const CHIR::FuncType& chirFuncTy, llvm::Function& llvmFunc) override;

protected:
    llvm::Type* GetStructReturnType(CHIR::StructType& chirTy, std::vector<llvm::Type*>& params) override;
    void ProcessInvocationArg(CHIR::StructType& chirParamTy, ProcessKind kind, size_t& argIdx,
        std::vector<CGValue*>& args, IRBuilder2& builder) override;

private:
    ProcessKind GetParamType(CHIR::Type& chirTy, std::vector<llvm::Type*>& params);
    ABIArgInfo GetMappingArgInfo(CHIR::StructType& chirTy, bool isArg);
};

class LinuxOhosArm32CODENativeCGCFFI : public LinuxAarch64CODENativeCGCFFI {
public:
    explicit LinuxOhosArm32CODENativeCGCFFI(CGModule& cgMod) : LinuxAarch64CODENativeCGCFFI(cgMod)
    {
    }

    llvm::FunctionType* GetCFuncType(const CHIR::FuncType& chirFuncTy) override;
    void ProcessParam(CHIR::Type& chirParamTy, LLVMFuncArgIt& arg, llvm::Value* place, IRBuilder2& builder) override;

protected:
    void ProcessInvocationArg(CHIR::StructType& chirParamTy, ProcessKind kind, size_t& argIdx,
        std::vector<CGValue*>& args, IRBuilder2& builder) override;
    llvm::Type* GetStructReturnType(CHIR::StructType& chirTy, std::vector<llvm::Type*>& params) override;

    std::unordered_map<Ptr<CHIR::Type>, ABIArgInfo> paramTypeMap;

private:
    ProcessKind GetParamType(CHIR::Type& chirTy, std::vector<llvm::Type*>& params);
    ABIArgInfo GetMappingArgInfo(CHIR::StructType& chirTy, bool isArg);
};

} // namespace CodeGen
} // namespace Codira
#endif
