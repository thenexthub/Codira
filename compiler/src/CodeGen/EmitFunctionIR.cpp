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

#include "EmitFunctionIR.h"

#include "Base/CGTypes/CGEnumType.h"
#include "CGModule.h"
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
#include "CODENative/CODENativeCGCFFI.h"
#endif
#include "DIBuilder.h"
#include "EmitBasicBlockIR.h"
#include "IRAttribute.h"
#include "IRBuilder.h"
#include "IRGenerator.h"
#include "Utils/CGCommonDef.h"
#include "Utils/CGUtils.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Value.h"

namespace {
using namespace Codira;
using namespace CodeGen;

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
inline bool NeedProcessParamOnDemand(const CGModule& cgMod, const CHIR::FuncType& chirFuncTy, size_t index)
{
    return cgMod.GetCGCFFI().NeedProcessParam(chirFuncTy, index);
}

inline llvm::Value* ProcessParamOnDemand(
    CGModule& cgMod, CHIR::Type& chirParamTy, llvm::Function::arg_iterator& arg, IRBuilder2& builder)
{
    auto cgType = CGType::GetOrCreate(cgMod, &chirParamTy);
    if (chirParamTy.IsVArray()) {
        auto paramType = cgType->GetLLVMType();
        return builder.CreateBitCast(arg, paramType->getPointerTo());
    }
    auto place = builder.CreateEntryAlloca(*cgType);
    cgMod.GetCGCFFI().ProcessParam(chirParamTy, arg, place, builder);
    return place;
}

inline void InitIRBuilder(
    std::unique_ptr<IRBuilder2>& builder, CGModule& cgMod, const CHIR::Func& chirFunc, llvm::Function& llvmFunc)
{
    if (!builder) {
        builder = std::make_unique<IRBuilder2>(cgMod);
        auto chirEntryBB = chirFunc.GetEntryBlock();
        auto entryBB = llvm::BasicBlock::Create(
            cgMod.GetLLVMContext(), "entry:" + chirEntryBB->GetIdentifierWithoutPrefix(), &llvmFunc);
        builder->SetInsertPoint(entryBB);
        cgMod.SetOrUpdateMappedBB(chirEntryBB, entryBB);
    }
}

void HandleCFuncParams(CGModule& cgMod, const CHIR::Func& chirFunc, llvm::Function& llvmFunc)
{
    auto chirFuncTy = chirFunc.GetFuncType();
    std::unique_ptr<IRBuilder2> builder;
    auto llvmArgIt = llvmFunc.arg_begin();
    if (chirFunc.GetNumOfParams() == 0) {
        return;
    }
    if (llvmArgIt != llvmFunc.arg_end() && llvmArgIt->hasStructRetAttr()) {
        ++llvmArgIt;
    }
    for (size_t chirArgIdx = 0; chirArgIdx < chirFunc.GetNumOfParams(); ++chirArgIdx) {
        CODEC_ASSERT(chirArgIdx < chirFunc.GetNumOfParams());
        auto chirFuncArg = chirFunc.GetParam(chirArgIdx);
        auto chirFuncArgTy = chirFuncArg->GetType();
        llvm::Value* llvmValue = nullptr;
        if (IsZeroSizedTypeInC(cgMod, *chirFuncArgTy)) {
            InitIRBuilder(builder, cgMod, chirFunc, llvmFunc);
            llvmValue = builder->CreateEntryAlloca(*CGType::GetOrCreate(cgMod, chirFuncArgTy));
        } else {
            CODEC_ASSERT(llvmArgIt != llvmFunc.arg_end());
            llvmValue = llvmArgIt;
            if (NeedProcessParamOnDemand(cgMod, *chirFuncTy, chirArgIdx)) {
                InitIRBuilder(builder, cgMod, chirFunc, llvmFunc);
                llvmValue = ProcessParamOnDemand(cgMod, *chirFuncArgTy, llvmArgIt, *builder);
            }
            ++llvmArgIt;
        }
        FixedCGTypeOfFuncArg(cgMod, *chirFuncArg, *llvmValue);
    }
}

inline void HandleCFuncParams(CGModule& cgMod, const CHIR::Value& chirFunc, llvm::Function& llvmFunc)
{
    if (chirFunc.IsFuncWithBody()) {
        HandleCFuncParams(cgMod, *VirtualCast<const CHIR::Func>(&chirFunc), llvmFunc);
    } else {
        // The remaining case can only be imported function, we needn't process params for it.
        CODEC_ASSERT(chirFunc.IsImportedFunc());
    }
}
#endif

void HandleFuncParams(CGModule& cgMod, const CHIR::Func& chirFunc, const CGFunction& cgFunc)
{
    for (size_t idx = 0; idx < chirFunc.GetNumOfParams(); ++idx) {
        auto chirFuncArg = chirFunc.GetParam(idx);
        auto llvmArg = cgFunc.GetArgByIndexFromCHIR(idx);
        auto fixedCGType = FixedCGTypeOfFuncArg(cgMod, *chirFuncArg, *llvmArg);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        if (chirFunc.IsConstructor() && idx == 0) {
            llvmArg->addAttr(llvm::Attribute::NoAlias);
        }
#endif
        if (fixedCGType->IsStructPtrType() || fixedCGType->IsVArrayPtrType()) {
            llvmArg->addAttr(llvm::Attribute::NoAlias);
        }
    }
}

inline void BuildCFunc(CGModule& cgMod, const CHIR::Value& chirFunc, const CGFunction& cgFunc, IRBuilder2& builder)
{
    CODEC_ASSERT(chirFunc.GetType()->IsFunc() && StaticCast<CHIR::FuncType*>(chirFunc.GetType())->IsCFunc());
    llvm::Function* llvmFunc = cgFunc.GetRawFunction();

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    auto chirFuncTy = StaticCast<CHIR::FuncType*>(chirFunc.GetType());
    cgMod.GetCGCFFI().AddFunctionAttr(*chirFuncTy, *llvmFunc);
#endif

    if (chirFunc.TestAttr(CHIR::Attribute::FOREIGN)) {
        return;
    }

    if (chirFunc.IsFuncWithBody()) {
        // CFFI wrap func should not set Personality
        auto funcNode = VirtualCast<const CHIR::Func*>(&chirFunc);
        if (!funcNode->IsCFFIWrapper()) {
            llvmFunc->setPersonalityFn(cgMod.GetExceptionIntrinsicPersonality());
        } else {
            auto scope = llvmFunc->getSubprogram();
            auto curLoc = builder.GetCGModule().diBuilder->CreateDILoc(scope, {scope ? scope->getLine() : 0, 0});
            builder.SetCurrentDebugLocation(curLoc);
        }
    } else {
        CODEC_ASSERT(chirFunc.IsImportedFunc());
    }
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    HandleCFuncParams(cgMod, chirFunc, *llvmFunc);
#endif
    return;
}
} // namespace

namespace Codira {
namespace CodeGen {
void BuildCODEFunc(CGModule& cgMod, const CHIR::Func& chirFunc, const CGFunction& cgFunc)
{
    llvm::Function* llvmFunc = cgFunc.GetRawFunction();
    llvmFunc->setPersonalityFn(cgMod.GetExceptionIntrinsicPersonality());
    HandleFuncParams(cgMod, chirFunc, cgFunc);
}

class FunctionGeneratorImpl : public IRGeneratorImpl {
    friend class CGModule;

public:
    explicit FunctionGeneratorImpl(CGModule& cgMod, const CHIR::Func& chirFunc) : cgMod(cgMod), chirFunc(chirFunc)
    {
    }

    void EmitIR() override;

private:
    CGModule& cgMod;
    const CHIR::Func& chirFunc;
};

template <> class IRGenerator<FunctionGeneratorImpl> : public IRGenerator<> {
public:
    IRGenerator(CGModule& cgMod, const CHIR::Func& chirFunc)
        : IRGenerator<>(std::make_unique<FunctionGeneratorImpl>(cgMod, chirFunc))
    {
    }
};

void FunctionGeneratorImpl::EmitIR()
{
    IRBuilder2 builder(cgMod);
    auto cgFunc = cgMod.GetOrInsertCGFunction(&chirFunc);
    auto rawFunction = cgFunc->GetRawFunction();
    bool isMockMode = (cgMod.GetCGContext().GetCompileOptions().mock == MockMode::ON) 
                      || cgMod.GetCGContext().GetCompileOptions().enableCompileTest;
    bool shouldGenDebugInfo = isMockMode || chirFunc.IsFuncWithBody();
    if(shouldGenDebugInfo) {
        cgMod.diBuilder->SetSubprogram(&chirFunc, rawFunction);
    }
    auto chirFuncTy = chirFunc.GetFuncType();
    CODEC_NULLPTR_CHECK(chirFuncTy);
    if (chirFuncTy->IsCFunc()) {
        CODEC_ASSERT(!chirFunc.TestAttr(CHIR::Attribute::FOREIGN));
        BuildCFunc(cgMod, chirFunc, *cgFunc, builder);
    } else {
        BuildCODEFunc(cgMod, chirFunc, *cgFunc);
    }

    if (!ShouldReturnVoid(chirFunc)) {
        SetSRetAttrForStructReturnType(*chirFuncTy, *rawFunction);
    }

    auto entryBB = chirFunc.GetBody()->GetEntryBlock();
    EmitBasicBlockIR(cgMod, *entryBB);
    if (rawFunction->getSubprogram()) {
        cgMod.GetCGContext().debugValue = nullptr;
        cgMod.diBuilder->FinalizeSubProgram(*rawFunction);
    }

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    const auto& options = cgMod.GetCGContext().GetCompileOptions();
    if ((options.enableTimer || options.enableMemoryCollect) && (chirFunc.GetGenericDecl() != nullptr)) {
        if (chirFunc.GetGenericDecl()->TestAttr(CHIR::Attribute::IMPORTED)) {
            cgFunc->GetRawFunction()->addFnAttr(GENERIC_DECL_IN_IMPORTED_PKG_ATTR);
        } else {
            cgFunc->GetRawFunction()->addFnAttr(GENERIC_DECL_IN_CURRENT_PKG_ATTR);
        }
    }

    if (chirFunc.IsLambda()) {
        cgFunc->GetRawFunction()->addFnAttr(FUNC_USED_BY_CLOSURE);
    }
#endif
}

void EmitFunctionIR(CGModule& cgMod, const CHIR::Func& chirFunc)
{
    IRGenerator<FunctionGeneratorImpl>(cgMod, chirFunc).EmitIR();
}

void EmitFunctionIR(CGModule& cgMod, const std::vector<CHIR::Func*>& chirFuncs)
{
    for (auto chirFunc : chirFuncs) {
        EmitFunctionIR(cgMod, *chirFunc);
    }
}

void EmitImportedCFuncIR(CGModule& cgMod, const std::vector<CHIR::ImportedFunc*>& importedCFuncs)
{
    IRBuilder2 builder(cgMod);
    for (auto importedCFunc : importedCFuncs) {
        auto chirType = importedCFunc->GetType();
        CODEC_ASSERT(chirType->IsFunc() && StaticCast<CHIR::FuncType*>(chirType)->IsCFunc());
        auto chirFuncTy = StaticCast<CHIR::FuncType*>(chirType);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        auto llvmFuncTy = cgMod.GetCGCFFI().GetCFuncType(*chirFuncTy);
#endif
        CODEC_NULLPTR_CHECK(llvmFuncTy);

        auto srcPkgName = importedCFunc->GetSourcePackageName();
        const auto& funcPkgName = srcPkgName.empty() ? cgMod.GetCGContext().GetCurrentPkgName() : srcPkgName;
        // Case 1: the CFunc is an implicitly imported one, we need to use its wrapper func when calling it.
        if (funcPkgName != cgMod.GetCGContext().GetCurrentPkgName()) {
            continue;
        }

        // Case 2: the CFunc is a foreign function.
        auto cgFunc = cgMod.GetOrInsertCGFunction(importedCFunc);
        BuildCFunc(cgMod, *importedCFunc, *cgFunc, builder);

        // Adaption cffi wrapper name for incremental compilation
        // If the package of an imported variable is the current package,
        // the imported variable is from the previous compilation product.
        if (cgMod.GetCGContext().GetCompileOptions().enIncrementalCompilation &&
            IsNonPublicCFunc(*chirFuncTy, *importedCFunc) && importedCFunc->TestAttr(CHIR::Attribute::NON_RECOMPILE)) {
            cgFunc->GetRawFunction()->addFnAttr(CodeGen::INCREMENTAL_CFUNC_ATTR);
            cgFunc->GetRawFunction()->addFnAttr(CodeGen::INTERNAL_CFUNC_ATTR);
        }

        SetSRetAttrForStructReturnType(*chirFuncTy, *cgFunc->GetRawFunction());
    }
}
} // namespace CodeGen
} // namespace Codira
