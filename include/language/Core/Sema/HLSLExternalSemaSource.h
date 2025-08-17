//===--- HLSLExternalSemaSource.h - HLSL Sema Source ------------*- C++ -*-===//
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
//  This file defines the HLSLExternalSemaSource interface.
//
//===----------------------------------------------------------------------===//
#ifndef CLANG_SEMA_HLSLEXTERNALSEMASOURCE_H
#define CLANG_SEMA_HLSLEXTERNALSEMASOURCE_H

#include "language/Core/Sema/ExternalSemaSource.h"
#include "toolchain/ADT/DenseMap.h"

namespace language::Core {
class NamespaceDecl;
class Sema;

class HLSLExternalSemaSource : public ExternalSemaSource {
  Sema *SemaPtr = nullptr;
  NamespaceDecl *HLSLNamespace = nullptr;

  using CompletionFunction = std::function<void(CXXRecordDecl *)>;
  toolchain::DenseMap<CXXRecordDecl *, CompletionFunction> Completions;

public:
  ~HLSLExternalSemaSource() override {}

  /// Initialize the semantic source with the Sema instance
  /// being used to perform semantic analysis on the abstract syntax
  /// tree.
  void InitializeSema(Sema &S) override;

  /// Inform the semantic consumer that Sema is no longer available.
  void ForgetSema() override { SemaPtr = nullptr; }

  using ExternalASTSource::CompleteType;
  /// Complete an incomplete HLSL builtin type
  void CompleteType(TagDecl *Tag) override;

private:
  void defineTrivialHLSLTypes();
  void defineHLSLVectorAlias();
  void defineHLSLTypesWithForwardDeclarations();
  void onCompletion(CXXRecordDecl *Record, CompletionFunction Fn);
};

} // namespace language::Core

#endif // CLANG_SEMA_HLSLEXTERNALSEMASOURCE_H
