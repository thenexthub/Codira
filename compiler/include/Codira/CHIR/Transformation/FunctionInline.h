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
 * This file declares FunctionInline class for CHIR
 */

#ifndef CODIRA_CHIR_TRANSFORMATION_FUNCTION_INLINE_H
#define CODIRA_CHIR_TRANSFORMATION_FUNCTION_INLINE_H

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Expression/Terminator.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/Option/Option.h"

namespace Codira::CHIR {
/**
 * CHIR Opt Pass: do function inline for CHIR IR.
 */
class FunctionInline {
public:
    /**
     * @brief constructor for function inline pass
     * @param builder CHIR builder for generating IR.
     * @param optLevel optimization level from Codira inputs.
     * @param debug flag whether print debug log.
     */
    FunctionInline(CHIRBuilder& builder, const GlobalOptions::OptimizationLevel& optLevel, bool debug)
        : builder(builder), optLevel(optLevel), debug(debug)
    {
    }

    /**
     * @brief Main process to do function inline.
     * @param func func to do function inline.
     */
    void Run(Func& func);

    /**
     * @brief Get effect map after this pass.
     * @return effect map affected by this pass.
     */
    const OptEffectCHIRMap& GetEffectMap() const;

    /**
     * @brief Main process to do function inline per apply.
     * @param apply apply expression to do function inline.
     * @param name pass name to pring log.
     */
    void DoFunctionInline(const Apply& apply, const std::string& name);

private:
    bool CheckCanRewrite(const Apply& apply);
    void RecordEffectMap(const Apply& apply);
    void ReplaceFuncResult(LocalVar* resNew, LocalVar* resOld);

    std::pair<BlockGroup*, LocalVar*> CloneBlockGroupForInline(
        const BlockGroup& other, Func& parentFunc, const Apply& apply);

    void SetGroupDebugLocation(BlockGroup& group, const DebugLocation& loc);

    void InlineImpl(BlockGroup& bg);

    CHIRBuilder& builder;
    const GlobalOptions::OptimizationLevel& optLevel;
    bool debug{false};
    Func* globalFunc{nullptr};
    std::unordered_map<Func*, size_t> inlinedCountMap;
    std::unordered_map<Func*, size_t> funcSizeMap;
    const std::string optName{"Function Inline"};
    OptEffectCHIRMap effectMap;
};
} // namespace Codira::CHIR

#endif
