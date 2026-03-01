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

#include "Codira/CHIR/Checker/ConstSafetyCheck.h"
#include "Codira/CHIR/Analysis/Engine.h"

using namespace Codira::CHIR;

ConstSafetyCheck::ConstSafetyCheck(ConstAnalysisWrapper* constAnalysisWrapper) : analysisWrapper(constAnalysisWrapper)
{
}

void ConstSafetyCheck::RunOnPackage(const Package& package, size_t threadNum) const
{
    if (threadNum == 1) {
        for (auto func : package.GetGlobalFuncs()) {
            RunOnFunc(func);
        }
    } else {
        Utils::TaskQueue taskQueue(threadNum);
        for (auto func : package.GetGlobalFuncs()) {
            taskQueue.AddTask<void>([this, func]() { return RunOnFunc(func); });
        }
        taskQueue.RunAndWaitForAllTasksCompleted();
    }
}

void ConstSafetyCheck::RunOnFunc(const Func* func) const
{
    auto result = analysisWrapper->CheckFuncResult(func);
    CODEC_ASSERT(result);

    const auto actionBeforeVisitExpr = [](const ConstDomain&, Expression*, size_t) {};
    const auto actionAfterVisitExpr = [](const ConstDomain&, Expression*, size_t) {};
    const auto actionOnTerminator = [](const ConstDomain&, Terminator*, std::optional<Block*>) {};

    result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
}
