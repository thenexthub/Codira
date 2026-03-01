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

#ifndef LIBLLVMBACKEND_LLVM_COMPILER_OPTIONS_H
#define LIBLLVMBACKEND_LLVM_COMPILER_OPTIONS_H

#include <cstdint>
#include <string>

namespace ark::llvmbackend {
struct LLVMCompilerOptions {
    // Options obtained using llvmbackend.yaml
    bool optimize;
    int optlevel;
    bool gcIntrusion;
    bool gcIntrusionChecks;
    bool inlining;
    bool recursiveInlining;
    bool useSafepoint;
    bool dumpModuleBeforeOptimizations;
    bool dumpModuleAfterOptimizations;
    std::string inlineModuleFile;
    std::string pipelineFile;
    // Internal options
    bool doIrtocInline;
    bool doVirtualInline;
};
}  // namespace ark::llvmbackend
#endif  // LIBLLVMBACKEND_LLVM_COMPILER_OPTIONS_H
