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

#ifndef CODIRA_CHIR_WRAP_VIRTUAL_FUNC_H
#define CODIRA_CHIR_WRAP_VIRTUAL_FUNC_H

#include <vector>

#include "Codira/IncrementalCompilation/CompilationCache.h"
#include "Codira/IncrementalCompilation/IncrementalScopeAnalysis.h"
#include "Codira/CHIR/UserDefinedType.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"

namespace Codira::CHIR {
class WrapVirtualFunc {
public:
    WrapVirtualFunc(CHIRBuilder& builder,
        const CompilationCache& increCachedInfo, const IncreKind incrementalKind, const bool targetIsWin);
    /**
    * @brief wrap virtual function
    *
    * @param customTypeDef wrap virtual function in this def's vtable
    */
    void CheckAndWrap(CustomTypeDef& customTypeDef);
    VirtualWrapperDepMap&& GetCurVirtFuncWrapDep();
    VirtualWrapperDepMap&& GetDelVirtFuncWrapForIncr();

private:
    struct WrapperFuncGenericTable {
        std::vector<GenericType*> funcGenericTypeParams;
        std::unordered_map<const GenericType*, Type*> replaceTable;
        std::unordered_map<const GenericType*, Type*> inverseReplaceTable;
    };
    FuncBase* CreateVirtualWrapperIfNeeded(const VirtualFuncInfo& funcInfo,
        const VirtualFuncInfo& parentFuncInfo, Type& selfTy, CustomTypeDef& customTypeDef, const ClassType& parentTy);
    void CreateVirtualWrapperFunc(Func& func, FuncType& wrapperTy,
        const VirtualFuncInfo& funcInfo, Type& selfTy, WrapVirtualFunc::WrapperFuncGenericTable& genericTable);
    WrapperFuncGenericTable GetReplaceTableForVirtualFunc(
        const ClassType& parentTy, const std::string& funcIdentifier, const VirtualFuncInfo& parentFuncInfo);
    FuncType* RemoveThisArg(FuncType* funcTy);
    void HandleVirtualFuncWrapperForIncrCompilation(const FuncBase* wrapper, const FuncBase& curFunc);
    FuncType* GetWrapperFuncType(FuncType& parentFuncTyWithoutThisArg,
        Type& selfTy, const std::unordered_map<const GenericType*, Type*>& replaceTable, bool isStatic);

private:
    CHIRBuilder& builder;
    const CompilationCache& increCachedInfo;
    const IncreKind incrementalKind;
    const bool targetIsWin;

    std::unordered_map<std::string, FuncBase*> wrapperCache;
    VirtualWrapperDepMap curVirtFuncWrapDep;
    VirtualWrapperDepMap delVirtFuncWrapForIncr;
};
}

#endif
