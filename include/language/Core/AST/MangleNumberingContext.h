//=== MangleNumberingContext.h - Context for mangling numbers ---*- C++ -*-===//
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
//  This file defines the LambdaBlockMangleContext interface, which keeps track
//  of the Itanium C++ ABI mangling numbers for lambda expressions and block
//  literals.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_AST_MANGLENUMBERINGCONTEXT_H
#define LANGUAGE_CORE_AST_MANGLENUMBERINGCONTEXT_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"

namespace language::Core {

class BlockDecl;
class CXXMethodDecl;
class TagDecl;
class VarDecl;

/// Keeps track of the mangled names of lambda expressions and block
/// literals within a particular context.
class MangleNumberingContext {
  // The index of the next lambda we encounter in this context.
  unsigned LambdaIndex = 0;

public:
  virtual ~MangleNumberingContext() {}

  /// Retrieve the mangling number of a new lambda expression with the
  /// given call operator within this context.
  virtual unsigned getManglingNumber(const CXXMethodDecl *CallOperator) = 0;

  /// Retrieve the mangling number of a new block literal within this
  /// context.
  virtual unsigned getManglingNumber(const BlockDecl *BD) = 0;

  /// Static locals are numbered by source order.
  virtual unsigned getStaticLocalNumber(const VarDecl *VD) = 0;

  /// Retrieve the mangling number of a static local variable within
  /// this context.
  virtual unsigned getManglingNumber(const VarDecl *VD,
                                     unsigned MSLocalManglingNumber) = 0;

  /// Retrieve the mangling number of a static local variable within
  /// this context.
  virtual unsigned getManglingNumber(const TagDecl *TD,
                                     unsigned MSLocalManglingNumber) = 0;

  /// Retrieve the mangling number of a new lambda expression with the
  /// given call operator within the device context. No device number is
  /// assigned if there's no device numbering context is associated.
  virtual unsigned getDeviceManglingNumber(const CXXMethodDecl *) { return 0; }

  // Retrieve the index of the next lambda appearing in this context, which is
  // used for deduplicating lambdas across modules. Note that this is a simple
  // sequence number and is not ABI-dependent.
  unsigned getNextLambdaIndex() { return LambdaIndex++; }
};

} // end namespace language::Core
#endif
