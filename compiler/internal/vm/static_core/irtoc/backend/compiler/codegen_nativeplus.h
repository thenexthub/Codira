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

#ifndef PANDA_CODEGEN_NATIVE_PLUS_H
#define PANDA_CODEGEN_NATIVE_PLUS_H

#include "codegen_fastpath.h"

namespace ark::compiler {

class CodegenNativePlus : public CodegenFastPath {
public:
    using CodegenFastPath::CodegenFastPath;

    explicit CodegenNativePlus(Graph *graph) : CodegenFastPath(graph) {}

    void CreateFrameInfo() override
    {
        Codegen::CreateFrameInfo();  // NOLINT(bugprone-parent-virtual-call)
    }

    void GeneratePrologue() override
    {
        Codegen::GeneratePrologue();  // NOLINT(bugprone-parent-virtual-call)
    }

    void GenerateEpilogue() override
    {
        Codegen::GenerateEpilogue();  // NOLINT(bugprone-parent-virtual-call
    }

    /* SlowPath intrinsic is not supported (yet) */
    void EmitSlowPathEntryIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override
    {
        Codegen::EmitSlowPathEntryIntrinsic(inst, dst, src);  // NOLINT(bugprone-parent-virtual-call
    }

    void EmitTailCallIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
};

}  // namespace ark::compiler

#endif  // PANDA_CODEGEN_NATIVE_PLUS_H
