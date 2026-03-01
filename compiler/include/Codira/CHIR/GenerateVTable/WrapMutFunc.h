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
 * This file generate mut func wrapper
 */

#ifndef CODIRA_CHIR_WRAP_MUT_FUNC_H
#define CODIRA_CHIR_WRAP_MUT_FUNC_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Type/ExtendDef.h"

namespace Codira {
namespace CHIR {
class WrapMutFunc {
public:
    WrapMutFunc(CHIRBuilder& b);

    /**
     * @brief Create wrapper func for mut method.
     *
     * @param customTypeDef Visit all mut methods in this CustomTypeDef.
     */
    void Run(CustomTypeDef& customTypeDef);

    /**
     * @brief Return cache info, map<mangled name, func pointer>.
     */
    std::unordered_map<std::string, FuncBase*>&& GetWrappers();

private:
    void CreateMutFuncWrapper(FuncBase* rawFunc, CustomTypeDef& curDef, ClassType& srcClassTy);

    CHIRBuilder& builder;
    std::unordered_map<std::string, FuncBase*> wrapperFuncs;
};
}
}
#endif
