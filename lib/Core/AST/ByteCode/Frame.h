//===--- Frame.h - Call frame for the VM and AST Walker ---------*- C++ -*-===//
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
//
// Defines the base class of interpreter and evaluator stack frames.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_FRAME_H
#define LANGUAGE_CORE_AST_INTERP_FRAME_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::Core {
class FunctionDecl;

namespace interp {

/// Base class for stack frames, shared between VM and walker.
class Frame {
public:
  virtual ~Frame() = default;

  /// Generates a human-readable description of the call site.
  virtual void describe(toolchain::raw_ostream &OS) const = 0;

  /// Returns a pointer to the caller frame.
  virtual Frame *getCaller() const = 0;

  /// Returns the location of the call site.
  virtual SourceRange getCallRange() const = 0;

  /// Returns the called function's declaration.
  virtual const FunctionDecl *getCallee() const = 0;
};

} // namespace interp
} // namespace language::Core

#endif
