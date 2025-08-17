//===-- lib/Semantics/check-cuda.h ------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_CUDA_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_CUDA_H_

#include "language/Compability/Semantics/semantics.h"
#include <list>

namespace language::Compability::parser {
struct Program;
class Messages;
struct Name;
class CharBlock;
struct AssignmentStmt;
struct ExecutionPartConstruct;
struct ExecutableConstruct;
struct ActionStmt;
struct IfConstruct;
struct CUFKernelDoConstruct;
struct SubroutineSubprogram;
struct FunctionSubprogram;
struct SeparateModuleSubprogram;
} // namespace language::Compability::parser

namespace language::Compability::semantics {

class SemanticsContext;

class CUDAChecker : public virtual BaseChecker {
public:
  explicit CUDAChecker(SemanticsContext &c) : context_{c} {}
  void Enter(const parser::SubroutineSubprogram &);
  void Enter(const parser::FunctionSubprogram &);
  void Enter(const parser::SeparateModuleSubprogram &);
  void Enter(const parser::CUFKernelDoConstruct &);
  void Leave(const parser::CUFKernelDoConstruct &);
  void Enter(const parser::AssignmentStmt &);
  void Enter(const parser::OpenACCBlockConstruct &);
  void Leave(const parser::OpenACCBlockConstruct &);
  void Enter(const parser::OpenACCCombinedConstruct &);
  void Leave(const parser::OpenACCCombinedConstruct &);
  void Enter(const parser::OpenACCLoopConstruct &);
  void Leave(const parser::OpenACCLoopConstruct &);
  void Enter(const parser::DoConstruct &);
  void Leave(const parser::DoConstruct &);

private:
  SemanticsContext &context_;
  int deviceConstructDepth_{0};
};

bool CanonicalizeCUDA(parser::Program &);

} // namespace language::Compability::semantics

#endif // FORTRAN_SEMANTICS_CHECK_CUDA_H_
