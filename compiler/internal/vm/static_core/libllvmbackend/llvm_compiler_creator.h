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

#ifndef LIBLLVMBACKEND_LLVM_COMPILER_CREATOR_H
#define LIBLLVMBACKEND_LLVM_COMPILER_CREATOR_H

#include "compiler_interface.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/utils/arch.h"

namespace ark::compiler {
class AotBuilder;
class LLVMAotBuilder;
class Graph;
class RuntimeInterface;
}  // namespace ark::compiler

namespace ark::llvmbackend {

std::unique_ptr<CompilerInterface> CreateLLVMAotCompiler(ark::compiler::RuntimeInterface *runtime,
                                                         ark::ArenaAllocator *allocator,
                                                         ark::compiler::LLVMAotBuilder *aotBuilder,
                                                         const std::string &cmdline, const std::string &filename);

std::unique_ptr<IrtocCompilerInterface> CreateLLVMIrtocCompiler(ark::compiler::RuntimeInterface *runtime,
                                                                ark::ArenaAllocator *allocator, ark::Arch arch);
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_COMPILER_CREATOR_H
