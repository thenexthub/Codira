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

#ifndef CODIRA_CHIR_TRANSFORMATION_MERGE_BLOCKS_H
#define CODIRA_CHIR_TRANSFORMATION_MERGE_BLOCKS_H

#include "Codira/Option/Option.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {
/**
 * CHIR Opt Pass: do block merge for CHIR IR.
 */
class MergeBlocks {
public:
    /**
     * @brief constructor for mergin blocks.
     */
    explicit MergeBlocks() = default;

    /**
     * @brief Main process to do block merge.
     * @param package package to do optimization.
     * @param builder CHIR builder for generating IR.
     * @param opts global options from Codira inputs.
     */
    static void RunOnPackage(const Package& package, CHIRBuilder& builder, const GlobalOptions& opts);

    /**
     * @brief Main process to do block merge per func.
     * @param body func body to merge blocks
     * @param builder CHIR builder for generating IR.
     * @param opts global options from Codira inputs.
     */
    static void RunOnFunc(const BlockGroup& body, CHIRBuilder& builder, const GlobalOptions& opts);
};
} // namespace Codira::CHIR

#endif
