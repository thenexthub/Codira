/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_CODEGEN_CODEGEN_NATIVE_H
#define COMPILER_OPTIMIZER_CODEGEN_CODEGEN_NATIVE_H

#include "optimizer/code_generator/codegen.h"

namespace ark::compiler {
/// CodegenNative provides support for 'Native' calling convention.
class CodegenNative : public Codegen {
public:
    explicit CodegenNative(Graph *graph) : Codegen(graph) {}
    NO_MOVE_SEMANTIC(CodegenNative);
    NO_COPY_SEMANTIC(CodegenNative);

    ~CodegenNative() override = default;

    const char *GetPassName() const override
    {
        return "CodegenNative";
    }
    void CreateFrameInfo() override;

protected:
    void GeneratePrologue() override;
    void GenerateEpilogue() override;
};  // CodegenNative
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_CODEGEN_NATIVE_H
