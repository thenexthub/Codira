//===----- SemaSwift.h --- Swift language-specific routines ---*- C++ -*---===//
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
/// \file
/// This file declares semantic analysis functions specific to Swift.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_SEMASWIFT_H
#define LANGUAGE_CORE_SEMA_SEMASWIFT_H

#include "language/Core/AST/ASTFwd.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Sema/SemaBase.h"

namespace language::Core {
class AttributeCommonInfo;
class Decl;
enum class ParameterABI;
class ParsedAttr;
class SwiftNameAttr;

class SemaSwift : public SemaBase {
public:
  SemaSwift(Sema &S);

  SwiftNameAttr *mergeNameAttr(Decl *D, const SwiftNameAttr &SNA,
                               StringRef Name);

  void handleAttrAttr(Decl *D, const ParsedAttr &AL);
  void handleAsyncAttr(Decl *D, const ParsedAttr &AL);
  void handleBridge(Decl *D, const ParsedAttr &AL);
  void handleError(Decl *D, const ParsedAttr &AL);
  void handleAsyncError(Decl *D, const ParsedAttr &AL);
  void handleName(Decl *D, const ParsedAttr &AL);
  void handleAsyncName(Decl *D, const ParsedAttr &AL);
  void handleNewType(Decl *D, const ParsedAttr &AL);

  /// Do a check to make sure \p Name looks like a legal argument for the
  /// swift_name attribute applied to decl \p D.  Raise a diagnostic if the name
  /// is invalid for the given declaration.
  ///
  /// \p AL is used to provide caret diagnostics in case of a malformed name.
  ///
  /// \returns true if the name is a valid swift name for \p D, false otherwise.
  bool DiagnoseName(Decl *D, StringRef Name, SourceLocation Loc,
                    const ParsedAttr &AL, bool IsAsync);
  void AddParameterABIAttr(Decl *D, const AttributeCommonInfo &CI,
                           ParameterABI abi);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_SEMA_SEMASWIFT_H
