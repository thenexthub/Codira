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
 * This file generate VTable in CHIR and update ir
 */

#ifndef CODIRA_CHIR_GENERATE_VTABLE_H
#define CODIRA_CHIR_GENERATE_VTABLE_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/IncrementalCompilation/IncrementalScopeAnalysis.h"
#include "Codira/Option/Option.h"

namespace Codira {
namespace CHIR {
class GenerateVTable {
public:
    GenerateVTable(Package& pkg, CHIRBuilder& b, const GlobalOptions& opts);

    /**
     * @brief Create VTable.
     */
    void CreateVTable();

    /**
     * @brief Update VTable for operator func.
     */
    void UpdateOperatorVirFunc();

    /**
     * @brief Create wrapper func for virtual method.
     *
     * @param kind The result of incremental compilation.
     * @param increCachedInfo Cache info of incremental compilation.
     * @param curVirtFuncWrapDep Dependency info, used by incremental compilation.
     * @param delVirtFuncWrapForIncr Dependency info, used by incremental compilation.
     */
    void CreateVirtualFuncWrapper(const IncreKind& kind, const CompilationCache& increCachedInfo,
        VirtualWrapperDepMap& curVirtFuncWrapDep, VirtualWrapperDepMap& delVirtFuncWrapForIncr);

    /**
     * @brief Create wrapper func for mut method.
     */
    void CreateMutFuncWrapper();

    /**
     * @brief Update Invoke and InvokeStatic after computing virtual func offset.
     */
    void UpdateFuncCall();

private:
    FuncBase* GetMutFuncWrapper(const Type& thisType, const std::vector<Value*>& args,
        const std::vector<Type*>& instTypeArgs, Type& retType, const FuncBase& callee);

    Package& package;
    CHIRBuilder& builder;
    const GlobalOptions& opts;
    std::unordered_map<std::string, FuncBase*> mutFuncWrappers;
};
}
}
#endif
