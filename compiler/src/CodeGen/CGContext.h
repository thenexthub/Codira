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

#ifndef CODIRA_CGCONTEXT_H
#define CODIRA_CGCONTEXT_H

#include <stack>
#include <unordered_set>

#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Value.h"

#include "Base/CGTypes/CGType.h"
#include "Base/CHIRExprWrapper.h"
#include "CGPkgContext.h"
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
#include "CODENative/CHIRSplitter.h"
#endif
#include "Utils/CGUtils.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/CHIRCasting.h"

namespace llvm::Intrinsic {
using ID = unsigned;
}

namespace Codira {
class GlobalOptions;

namespace CHIR {
class CHIRBuilder;
class ImportedValue;
} // namespace CHIR

namespace CodeGen {
class CGContextImpl;
class CGFunction;
class CGValue;
class CGEnumLayout;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
class CGCFFI;
#endif
struct CallBaseToReplaceInfo {
    llvm::CallBase* callWithoutTI;
    CHIRApplyWrapper applyExprW;

    CallBaseToReplaceInfo(llvm::CallBase* callWithoutTI, const CHIRApplyWrapper& applyExprW)
        : callWithoutTI(callWithoutTI), applyExprW(applyExprW)
    {
    }
};
class CGContext {
    friend class CGModule;
    friend class CGType;
    friend class CGFunctionType;

public:
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    explicit CGContext(const SubCHIRPackage& subCHIRPackage, CGPkgContext& cgPkgContext);
#endif
    ~CGContext();
    void Clear();

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    const SubCHIRPackage& GetSubCHIRPackage() const
    {
        return subCHIRPackage;
    }
#endif
    llvm::StructType* GetCodeStringType() const;

    CGPkgContext& GetCGPkgContext() const
    {
        return cgPkgContext;
    }

    CHIR::CHIRBuilder& GetCHIRBuilder() const
    {
        return cgPkgContext.GetCHIRBuilder();
    }

    const CHIR::Package& GetCHIRPackage() const
    {
        return cgPkgContext.GetCHIRPackage();
    }

    std::string GetCurrentPkgName() const
    {
        return cgPkgContext.GetCurrentPkgName();
    }

    const GlobalOptions& GetCompileOptions() const
    {
        return cgPkgContext.GetGlobalOptions();
    }

    llvm::LLVMContext& GetLLVMContext()
    {
        return *llvmContext;
    }

    CHIR::FuncBase* GetImplicitUsedFunc(const std::string& funcMangledName)
    {
        return cgPkgContext.GetImplicitUsedFunc(funcMangledName);
    }

    const CachedMangleMap& GetCachedMangleMap() const
    {
        return cgPkgContext.GetCachedMangleMap();
    }

    bool IsCGParallelEnabled() const
    {
        return cgPkgContext.IsCGParallelEnabled();
    }

    bool IsLineInfoEnabled() const
    {
        return cgPkgContext.IsLineInfoEnabled();
    }

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    bool IsCustomTypeOfOtherLLVMModule(const CHIR::Type& chirType)
    {
        if (auto customType = DynamicCast<const CHIR::CustomType*>(&chirType)) {
            if (customType->GetCustomTypeDef()->TestAttr(CHIR::Attribute::IMPORTED)) {
                return true;
            }
            auto customDef = customType->GetCustomTypeDef();
            return subCHIRPackage.chirCustomDefs.find(customDef) == subCHIRPackage.chirCustomDefs.end();
        }
        return false;
    }

    bool IsValueOfOtherLLVMModule(const CHIR::Value& chirValue)
    {
        if (chirValue.TestAttr(CHIR::Attribute::IMPORTED)) {
            return true;
        }

        if (chirValue.IsFuncWithBody()) {
            auto chirFunc = const_cast<CHIR::Func*>(DynamicCast<const CHIR::Func*>(&chirValue));
            return subCHIRPackage.chirFuncs.find(chirFunc) == subCHIRPackage.chirFuncs.end();
        } else if (chirValue.IsGlobalVarInCurPackage()) {
            auto chirGV = const_cast<CHIR::GlobalVar*>(DynamicCast<const CHIR::GlobalVar*>(&chirValue));
            return subCHIRPackage.chirGVs.find(chirGV) == subCHIRPackage.chirGVs.end();
        }
        return false;
    }

    void AddCallBaseToInline(llvm::CallBase* callBase, llvm::ReturnInst* retInst)
    {
        callBasesToInline.emplace_back(callBase, retInst);
    }

    const std::vector<std::pair<llvm::CallBase*, llvm::ReturnInst*>>& GetCallBasesToInline() const
    {
        return callBasesToInline;
    }

    void AddCallBaseToReplace(llvm::CallBase* callWithoutTI, const CHIRApplyWrapper& applyExprW)
    {
        auto tmp = CallBaseToReplaceInfo(callWithoutTI, applyExprW);
        callBasesToReplace.emplace_back(tmp);
    }

    const std::vector<CallBaseToReplaceInfo>& GetCallBasesToReplace() const
    {
        return callBasesToReplace;
    }

    void AddDebugLocOfRetExpr(llvm::Function* func, CHIR::DebugLocation debugLoc)
    {
        debugLocOfRetExpr[func].emplace_back(debugLoc);
    }

    const std::vector<CHIR::DebugLocation>& GetDebugLocOfRetExpr(llvm::Function* func)
    {
        return debugLocOfRetExpr[func];
    }

    void AddLocalizedSymbol(const std::string& symName)
    {
        cgPkgContext.AddLocalizedSymbol(symName);
    }
#endif

    void AddCODEString(const std::string& codeStringName, const std::string& codeStringContent)
    {
        codeStrings.emplace(std::make_pair(codeStringName, codeStringContent));
    }

    const std::unordered_map<std::string, std::string>& GetCODEStrings() const
    {
        return codeStrings;
    }

    void AddCodeGenAddedFuncsOrVars(const CHIR::Type& ty, std::string& codegenAddedName)
    {
        if (!cgPkgContext.GetGlobalOptions().enIncrementalCompilation) {
            return;
        }
        if (ty.IsClass() || ty.IsStruct() || ty.IsEnum()) {
            auto declMangledName = StaticCast<CHIR::CustomType&>(ty).GetCustomTypeDef()->GetIdentifierWithoutPrefix();
            codegenAddedFuncsOrVars[declMangledName].insert(codegenAddedName);
        }
    }

    const std::unordered_map<std::string, std::unordered_set<std::string>>& GetCodeGenAddedFuncsOrVars() const
    {
        return codegenAddedFuncsOrVars;
    }

    void PushUnwindBlockStack(llvm::BasicBlock* unwindBlock);
    std::optional<llvm::BasicBlock*> TopUnwindBlockStack() const;
    void PopUnwindBlockStack();

    void AddGeneratedStructType(const std::string& structTypeName);
    const std::set<std::string>& GetGeneratedStructType() const;
    bool IsGeneratedStructType(const std::string& structTypeName);

    void AddGlobalsOfCompileUnit(const std::string& globalsName);
    bool IsGlobalsOfCompileUnit(const std::string& globalsName);

    void RegisterStaticGIName(llvm::StringRef staticGIName)
    {
        (void)staticGINames.emplace(staticGIName.str());
    }
    const std::set<std::string>& GetStaticGINames() const
    {
        return staticGINames;
    }

    void RegisterReflectGeneratedStaticGIName(std::string staticGIName)
    {
        (void)reflectGeneratedStaticGINames.emplace_back(staticGIName);
    }

    const std::vector<std::string>& GetReflectGeneratedStaticGINames() const
    {
        return reflectGeneratedStaticGINames;
    }

    static std::vector<std::string> GetTINameArrayForUpperBounds(const std::vector<CHIR::Type*>& upperBounds)
    {
        std::vector<std::string> res;
        for (auto upperBound : upperBounds) {
            res.emplace_back(CGType::GetNameOfTypeInfoGV(*DeRef(*upperBound)));
        }
        return res;
    }

    std::string GetGenericTypeUniqueName(std::string& genericTypeName, const std::vector<CHIR::Type*>& upperBounds)
    {
        if (genericTypeWithUpperBoundsMap.find(genericTypeName) != genericTypeWithUpperBoundsMap.end()) {
            auto upperBoundsVec = genericTypeWithUpperBoundsMap[genericTypeName];
            for (size_t idx = 0; idx < upperBoundsVec.size(); idx++) {
                if (upperBoundsVec[idx] == GetTINameArrayForUpperBounds(upperBounds)) {
                    return genericTypeName + "." + std::to_string(idx);
                }
            }
            genericTypeWithUpperBoundsMap[genericTypeName].emplace_back(GetTINameArrayForUpperBounds(upperBounds));
            return genericTypeName + "." + std::to_string(upperBoundsVec.size());
        } else {
            genericTypeWithUpperBoundsMap[genericTypeName].emplace_back(GetTINameArrayForUpperBounds(upperBounds));
            return genericTypeName + ".0";
        }
        CODEC_ASSERT(false && "should not reach here");
        return "";
    }

    void AddDependentPartialOrderOfTypes(llvm::Constant* smaller, llvm::Constant* bigger)
    {
        if (dependentPartialOrderOfTypes.emplace(CGContext::PartialOrderPair{smaller, bigger}).second) {
            ++indegreeOfTypes[bigger];
        }
    }

    struct PartialOrderPair {
        llvm::Constant* smaller;
        llvm::Constant* bigger;
        bool operator<(const PartialOrderPair& that) const
        {
            if (this->smaller < that.smaller) {
                return true;
            } else if (this->smaller == that.smaller) {
                return this->bigger < that.bigger;
            } else {
                return false;
            }
        }
    };

    const std::set<PartialOrderPair>& GetDependentPartialOrderOfTypes() const
    {
        return dependentPartialOrderOfTypes;
    }

    const std::unordered_map<llvm::Constant*, size_t>& GetIndegreeOfTypes() const
    {
        return indegreeOfTypes;
    }

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    void AddLLVMUsedVars(const std::string& mangledNameOfGV)
    {
        llvmUsedGVs.emplace(mangledNameOfGV);
    }
    const std::set<std::string>& GetLLVMUsedVars() const
    {
        return llvmUsedGVs;
    }

    void AddNullableReference(llvm::Value* value);

    void SetBasePtr(const llvm::Value* val, llvm::Value* basePtr);
    llvm::Value* GetBasePtrOf(llvm::Value* val) const;

    void SetBoxedValueMap(llvm::Value* boxedRefVal, llvm::Value* originalNonRefVal)
    {
        (void)nonRefBox2RefMap.emplace(boxedRefVal, originalNonRefVal);
    }

    llvm::Value* GetOriginalNonRefValOfBoxedValue(llvm::Value* boxedRefVal) const
    {
        auto itor = nonRefBox2RefMap.find(boxedRefVal);
        if (itor != nonRefBox2RefMap.end()) {
            return itor->second;
        } else {
            return nullptr;
        }
    }
#endif

    void Add2CGTypePool(CGType* cgType);
    uint16_t GetVTableSizeOf(const CHIR::ClassType* introType)
    {
        auto classDef = introType->GetClassDef();
        if (classDef->IsInterface()) {
            return 0U;
        }
        if (auto it = vtableSizeMap.find(classDef); it != vtableSizeMap.end()) {
            return it->second;
        }
        uint16_t cnt = 0U;
        auto superDef = classDef;
        while (superDef) {
            ++cnt;
            superDef = superDef->GetSuperClassDef();
        }
        vtableSizeMap.emplace(classDef, cnt);
        return cnt;
    }

    CGPkgContext& cgPkgContext;
    llvm::Value* debugValue = nullptr;

    std::unordered_map<llvm::Value*, CGFunction*> function2CGFunc;
    std::unordered_map<llvm::Function*, std::unordered_map<const CHIR::Type*, llvm::Value*>> genericParamsCacheMap;
    std::unordered_map<llvm::BasicBlock*, std::unordered_map<llvm::Value*, llvm::Value*>>
        genericParamsSizeBlockLevelCacheMap;
    std::unordered_map<const CHIR::ClassDef*, uint16_t> vtableSizeMap;

private:
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    SubCHIRPackage subCHIRPackage;
#endif
    llvm::LLVMContext* llvmContext;
    std::unique_ptr<CGContextImpl> impl;
    std::stack<llvm::BasicBlock*> unwindBlockStack;
    // llvm::StructType used by subModule to generate for_keeping_some_types.
    std::set<std::string> generatedStructType;
    std::set<std::string> globalsOfCompileUnit;
    std::set<std::string> usedLLVMStructTypes;
    std::set<PartialOrderPair> dependentPartialOrderOfTypes;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    // When HotReload is enabled, we should put those user-defined GVs or non-param constructors with `internal`
    // linkage into llvm.used to prevent them from being eliminated by llvm-opt.
    std::set<std::string> llvmUsedGVs;
    std::set<std::string> staticGINames;
    std::vector<std::string> reflectGeneratedStaticGINames;
    std::vector<std::pair<llvm::CallBase*, llvm::ReturnInst*>> callBasesToInline;
    std::vector<CallBaseToReplaceInfo> callBasesToReplace;
#endif
    // Key: the generic type's srcCodeIdentifier
    // Value: {upperbounds vector1, upperbounds vector2, ...}
    std::unordered_map<std::string, std::vector<std::vector<std::string>>> genericTypeWithUpperBoundsMap;
    // Key: The global variable's name of the codeString
    // Value: Store the content of codeString
    std::unordered_map<std::string, std::string> codeStrings;
    std::unordered_map<llvm::Constant*, size_t> indegreeOfTypes;
    std::unordered_map<std::string, std::unordered_set<std::string>> codegenAddedFuncsOrVars;
    std::unordered_map<const CHIR::EnumDef*, std::vector<llvm::Constant*>> enumInfoCache;
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    std::map<llvm::Function*, std::vector<CHIR::DebugLocation>> debugLocOfRetExpr;
    // Key: boxed ref value; Value: original non-ref value
    std::unordered_map<llvm::Value*, llvm::Value*> nonRefBox2RefMap;
#endif
};
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_CGCONTEXT_H
