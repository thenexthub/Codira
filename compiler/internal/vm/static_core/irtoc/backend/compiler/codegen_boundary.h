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

#ifndef PANDA_CODEGEN_BOUNDARY_H
#define PANDA_CODEGEN_BOUNDARY_H

#include "compiler/optimizer/code_generator/codegen.h"

namespace ark::compiler {

/// Code generation for code living within Boundary Frame
class CodegenBoundary : public Codegen {
public:
    using Codegen::Codegen;

    void GeneratePrologue() override;
    void GenerateEpilogue() override;

    void CreateFrameInfo() override;

    void EmitTailCallIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;

private:
    void RemoveBoundaryFrame(const BasicBlock *bb) const;
};

}  // namespace ark::compiler

#endif  // PANDA_CODEGEN_BOUNDARY_H
