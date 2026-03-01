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

#include "Codira/CHIR/Transformation/RedundantGetOrThrowElimination.h"

#include "Codira/CHIR/Analysis/Engine.h"
#include "Codira/CHIR/Analysis/GetOrThrowResultAnalysis.h"
#include "Codira/CHIR/Analysis/Utils.h"

using namespace Codira::CHIR;

RedundantGetOrThrowElimination::RedundantGetOrThrowElimination()
{
}

void RedundantGetOrThrowElimination::RunOnPackage(const Ptr<const Package>& package, bool isDebug) const
{
    for (auto func : package->GetGlobalFuncs()) {
        RunOnFunc(func, isDebug);
    }
}

void RedundantGetOrThrowElimination::RunOnFunc(const Ptr<const Func>& func, bool isDebug) const
{
    auto analysis = std::make_unique<GetOrThrowResultAnalysis>(func, isDebug);
    auto engine = Engine<GetOrThrowResultDomain>(func, std::move(analysis));
    auto result = engine.IterateToFixpoint();
    CODEC_NULLPTR_CHECK(result);

    const auto actionBeforeVisitExpr = [func, isDebug](const GetOrThrowResultDomain& state, Expression* expr, size_t) {
        if (!IsGetOrThrowFunction(*expr)) {
            return;
        }
        auto apply = StaticCast<Apply*>(expr);
        auto arg = apply->GetArgs()[0];
        if (auto result = state.CheckGetOrThrowResult(arg); result) {
            apply->GetResult()->ReplaceWith(*result->GetResult(), func->GetBody());
            if (isDebug) {
                std::string message = "[RGetOtThrowE] The usages of the result of getOrThrow" +
                    ToPosInfo(apply->GetDebugLocation()) + " have been replaced by the value" +
                    ToPosInfo(result->GetDebugLocation()) + "\n";
                std::cout << message;
            }
        }
    };

    const auto actionAfterVisitExpr = [](const GetOrThrowResultDomain&, Expression*, size_t) {};

    const auto actionOnTerminator = [](const GetOrThrowResultDomain&, Terminator*, std::optional<Block*>) {};

    result->VisitWith(actionBeforeVisitExpr, actionAfterVisitExpr, actionOnTerminator);
}
