//===--- SemaConsumer.h - Abstract interface for AST semantics --*- C++ -*-===//
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
//  This file defines the SemaConsumer class, a subclass of
//  ASTConsumer that is used by AST clients that also require
//  additional semantic analysis.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_SEMA_SEMACONSUMER_H
#define LANGUAGE_CORE_SEMA_SEMACONSUMER_H

#include "language/Core/AST/ASTConsumer.h"

namespace language::Core {
  class Sema;

  /// An abstract interface that should be implemented by
  /// clients that read ASTs and then require further semantic
  /// analysis of the entities in those ASTs.
  class SemaConsumer : public ASTConsumer {
    virtual void anchor();
  public:
    SemaConsumer() {
      ASTConsumer::SemaConsumer = true;
    }

    /// Initialize the semantic consumer with the Sema instance
    /// being used to perform semantic analysis on the abstract syntax
    /// tree.
    virtual void InitializeSema(Sema &S) {}

    /// Inform the semantic consumer that Sema is no longer available.
    virtual void ForgetSema() {}

    // isa/cast/dyn_cast support
    static bool classof(const ASTConsumer *Consumer) {
      return Consumer->SemaConsumer;
    }
  };
}

#endif
