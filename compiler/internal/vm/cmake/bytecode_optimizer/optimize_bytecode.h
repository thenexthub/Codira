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

#ifndef BYTECODE_OPTIMIZER_OPTIMIZE_BYTECODE_H
#define BYTECODE_OPTIMIZER_OPTIMIZE_BYTECODE_H

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-program.h"
#include "compiler/optimizer/ir/graph.h"
#include "ir_interface.h"

namespace panda::bytecodeopt {
bool AnalysisBytecode(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const std::string &pandafile_name, bool is_dynamic = false, bool has_memory_pool = false);
bool RunOptimizations(compiler::Graph *graph, BytecodeOptIrInterface *iface = nullptr);
bool OptimizeBytecode(pandasm::Program *prog, const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                      const std::string &pandafile_name, bool is_dynamic = false, bool has_memory_pool = false);
}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_OPTIMIZE_BYTECODE_H
