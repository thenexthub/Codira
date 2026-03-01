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

#ifndef LIBLLVMBACKEND_LOWERING_DEBUG_DATA_BUILDER_H
#define LIBLLVMBACKEND_LOWERING_DEBUG_DATA_BUILDER_H

#include <string>
#include <memory>
#include "libarkbase/macros.h"

namespace llvm {
class Function;
class DIBuilder;
class DICompileUnit;
class Instruction;
class Module;
}  // namespace llvm

namespace ark::llvmbackend {

class DebugDataBuilder final {
public:
    explicit DebugDataBuilder(llvm::Module *module, const std::string &filename);

    void BeginSubprogram(llvm::Function *function, const std::string &filename, uint32_t line);
    void EndSubprogram(llvm::Function *function);

    void SetLocation(llvm::Instruction *inst, uint32_t line, uint32_t column = 1);

    void AppendInlinedAt(llvm::Instruction *inst, llvm::Function *function, uint32_t line, uint32_t column = 1);

    void Finalize();

    ~DebugDataBuilder();

    NO_COPY_SEMANTIC(DebugDataBuilder);
    NO_MOVE_SEMANTIC(DebugDataBuilder);

private:
    llvm::DIBuilder *builder_;
};

}  // namespace ark::llvmbackend

#endif  // LIBLLVMBACKEND_LOWERING_DEBUG_DATA_BUILDER_H
