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

#include "evaluation/evaluation_engine.h"

#include "include/tooling/pt_thread.h"

namespace ark::tooling::inspector {
Expected<std::pair<TypedValue, ObjectHeader *>, Error> PtThreadEvaluationEngine::EvaluateExpression(
    uint32_t frameNumber, const ExpressionWrapper &bytecode, Method **method)
{
    ASSERT(!IsEvaluating());
    ASSERT(method != nullptr);

    VRegValue result;
    std::optional<Error> err;

    evaluating_ = true;
    if (*method != nullptr) {
        err = debugger_->EvaluateExpression(PtThread(thread_), frameNumber, *method, &result);
    } else {
        err = debugger_->EvaluateExpression(PtThread(thread_), frameNumber, bytecode, method, &result);
    }
    evaluating_ = false;

    if (err) {
        return Unexpected(*err);
    }
    ASSERT(*method != nullptr);

    auto *exception = thread_->GetException();
    // Current implementation must clear any occurred exceptions
    if (exception != nullptr) {
        thread_->SetException(nullptr);
    }

    return std::make_pair(result.ToTypedValue((*method)->GetReturnType().GetId()), exception);
}
}  // namespace ark::tooling::inspector
