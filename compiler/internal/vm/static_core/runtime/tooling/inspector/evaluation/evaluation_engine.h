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

#ifndef PANDA_TOOLING_INSPECTOR_EVALUATION_EVALUATION_ENGINE_H
#define PANDA_TOOLING_INSPECTOR_EVALUATION_EVALUATION_ENGINE_H

#include "include/managed_thread.h"
#include "include/object_header.h"
#include "include/tooling/debug_interface.h"

namespace ark::tooling::inspector {
class EvaluationEngine {
public:
    EvaluationEngine() = default;

    NO_COPY_SEMANTIC(EvaluationEngine);
    NO_MOVE_SEMANTIC(EvaluationEngine);

    virtual ~EvaluationEngine() = default;

    virtual Expected<std::pair<TypedValue, ObjectHeader *>, Error> EvaluateExpression(uint32_t frameNumber,
                                                                                      const ExpressionWrapper &bytecode,
                                                                                      Method **method) = 0;
};

/// @brief Class provides debugger-based evaluation within the given application thread.
class PtThreadEvaluationEngine : public EvaluationEngine {
public:
    explicit PtThreadEvaluationEngine(DebugInterface *debugger, ManagedThread *thread)
        : debugger_(debugger), thread_(thread)
    {
        ASSERT(debugger_ != nullptr);
        ASSERT(thread_ != nullptr);
    }

    NO_COPY_SEMANTIC(PtThreadEvaluationEngine);
    NO_MOVE_SEMANTIC(PtThreadEvaluationEngine);

    ~PtThreadEvaluationEngine() override = default;

    ManagedThread *GetManagedThread()
    {
        return thread_;
    }

    bool IsEvaluating() const
    {
        return evaluating_;
    }

    /**
     * @brief Evaluates the given bytecode expression.
     * @param frameNumber frame depth to evaluate expression in.
     * @param bytecode fragment with expression.
     * @param method pointer either providing the previously loaded method or used for storing the expression method.
     * @returns pair of result and raised exception or an error.
     */
    Expected<std::pair<TypedValue, ObjectHeader *>, Error> EvaluateExpression(uint32_t frameNumber,
                                                                              const ExpressionWrapper &bytecode,
                                                                              Method **method) final;

private:
    DebugInterface *debugger_;
    ManagedThread *thread_;
    bool evaluating_ {false};
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_EVALUATION_EVALUATION_ENGINE_H
