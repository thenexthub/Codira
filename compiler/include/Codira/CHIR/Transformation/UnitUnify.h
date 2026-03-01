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

#ifndef CODIRA_CHIR_TRANSFORMATION_UNIT_UNIFY_H
#define CODIRA_CHIR_TRANSFORMATION_UNIT_UNIFY_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
/**
 * @brief unify all used units to one in a function.
 *    eliminate side effects for unit variables
 *
 * before pass:
 *     %0: Unit = Apply(@_CN7default3fooEv)
 *     %1: Void = Apply(@_CN7default3foo2Ev, %0)
 *     %2: Void = Apply(@_CN7default3foo3Ev, %0)
 * after pass:
 *     %3: Unit = Constant(unit)
 *     %0: Unit = Apply(@_CN7default3fooEv)
 *     %1: Void = Apply(@_CN7default3foo2Ev, %3)  // change used unit to const value %3
 *     %2: Void = Apply(@_CN7default3foo3Ev, %3)  // change used unit to const value %3
 */
class UnitUnify {
public:
    /**
     * @brief constructor to unify all used units to one in a function.
     * @param builder CHIR builder for generating IR.
     */
    explicit UnitUnify(CHIRBuilder& builder);

    /**
     * @brief Main process to unify all used units to one in a function.
     * @param package package to do optimization.
     * @param isDebug flag whether print debug log.
     */
    void RunOnPackage(const Ptr<const Package>& package, bool isDebug);

private:
    void RunOnFunc(const Ptr<Func>& func, bool isDebug);

    void LoadOrCreateUnit(Ptr<Constant>& constant, const Ptr<BlockGroup>& group);

    CHIRBuilder& builder;
};
} // namespace Codira::CHIR

#endif
