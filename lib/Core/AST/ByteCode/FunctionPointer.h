//===--- FunctionPointer.h - Types for the constexpr VM ---------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_INTERP_FUNCTION_POINTER_H
#define LANGUAGE_CORE_AST_INTERP_FUNCTION_POINTER_H

#include "Function.h"
#include "Primitives.h"

namespace language::Core {
class ASTContext;
class APValue;
namespace interp {

class FunctionPointer final {
private:
  const Function *Func;

public:
  FunctionPointer() = default;
  FunctionPointer(const Function *Func) : Func(Func) {}

  const Function *getFunction() const { return Func; }
  bool isZero() const { return !Func; }
  bool isWeak() const {
    if (!Func || !Func->getDecl())
      return false;

    return Func->getDecl()->isWeak();
  }

  APValue toAPValue(const ASTContext &) const;
  void print(toolchain::raw_ostream &OS) const;

  std::string toDiagnosticString(const ASTContext &Ctx) const {
    if (!Func)
      return "nullptr";

    return toAPValue(Ctx).getAsString(Ctx, Func->getDecl()->getType());
  }

  uint64_t getIntegerRepresentation() const {
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(Func));
  }
};

} // namespace interp
} // namespace language::Core

#endif
