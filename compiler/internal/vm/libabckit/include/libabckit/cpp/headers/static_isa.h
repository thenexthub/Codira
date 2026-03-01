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

#ifndef CPP_ABCKIT_STATIC_ISA_H
#define CPP_ABCKIT_STATIC_ISA_H

#include <memory>
#include "instruction.h"

namespace abckit {

class Graph;

/**
 * @brief DynamicIsa class containing API's for dynamic ISA manipulation
 */
class StaticIsa final {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief To access private constructor
    friend class Graph;

public:
    /**
     * @brief Deleted constructor
     * @param other
     */
    StaticIsa(const StaticIsa &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     */
    StaticIsa &operator=(const StaticIsa &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     */
    StaticIsa(StaticIsa &&other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     */
    StaticIsa &operator=(StaticIsa &&other) = delete;

    /**
     * @brief Destructor
     */
    ~StaticIsa() = default;

    /**
     * @brief Returns opcode of `inst`.
     * @return enum value of `AbckitIsaApiStaticOpcode`.
     * @param [ in ] inst - Inst to be inspected.
     */
    AbckitIsaApiStaticOpcode GetOpCode(Instruction inst) &&;

private:
    // All static methods impl
    explicit StaticIsa(const Graph &graph) : graph_(graph) {};
    // NOTE(nsizov): remove maybe unused, use graph_
    [[maybe_unused]] const Graph &graph_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_STATIC_ISA_H
