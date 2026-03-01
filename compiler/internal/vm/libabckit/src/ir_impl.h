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

#ifndef LIBABCKIT_SRC_IR_IMPL_H
#define LIBABCKIT_SRC_IR_IMPL_H

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "macros.h"
#include "libpandabase/macros.h"

namespace ark::compiler {
class BasicBlock;
class Inst;
class Graph;
}  // namespace ark::compiler

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct AbckitIrInterface final {
    AbckitIrInterface(std::unordered_map<uint32_t, std::string> methodsArg,
                      std::unordered_map<uint32_t, std::string> fieldsArg,
                      std::unordered_map<uint32_t, std::string> classesArg,
                      std::unordered_map<uint32_t, std::string> stringsArg,
                      std::unordered_map<uint32_t, std::string> literalarraysArg)
        : methods(std::move(methodsArg)),
          fields(std::move(fieldsArg)),
          classes(std::move(classesArg)),
          strings(std::move(stringsArg)),
          literalarrays(std::move(literalarraysArg))
    {
    }

    std::string GetMethodIdByOffset(uint32_t offset) const;
    std::string GetStringIdByOffset(uint32_t offset) const;
    std::string GetLiteralArrayIdByOffset(uint32_t offset) const;
    std::string GetTypeIdByOffset(uint32_t offset) const;
    std::string GetFieldIdByOffset(uint32_t offset) const;

    std::unordered_map<uint32_t, std::string> methods;
    std::unordered_map<uint32_t, std::string> fields;
    std::unordered_map<uint32_t, std::string> classes;
    std::unordered_map<uint32_t, std::string> strings;
    std::unordered_map<uint32_t, std::string> literalarrays;
};

struct AbckitInst {
    AbckitGraph *graph = nullptr;
    ark::compiler::Inst *impl = nullptr;

    AbckitInst(AbckitGraph *graph, ark::compiler::Inst *impl) : graph(graph), impl(impl) {}
    AbckitInst() = default;
    DEFAULT_COPY_SEMANTIC(AbckitInst);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(AbckitInst);
    ~AbckitInst() = default;
};

struct AbckitBasicBlock {
    AbckitGraph *graph;
    ark::compiler::BasicBlock *impl;
};

struct AbckitGraph {
    AbckitFile *file = nullptr;
    AbckitCoreFunction *function = nullptr;
    AbckitIrInterface *irInterface = nullptr;
    std::unordered_map<ark::compiler::BasicBlock *, AbckitBasicBlock *> implToBB;
    std::unordered_map<ark::compiler::Inst *, AbckitInst *> implToInst;
    std::unordered_map<uintptr_t, AbckitCoreClass *> ptrToClass;
    ark::compiler::Graph *impl = nullptr;
    void *internal = nullptr;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

#endif  // LIBABCKIT_SRC_IR_IMPL_H
