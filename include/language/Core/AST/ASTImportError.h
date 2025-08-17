//===- ASTImportError.h - Define errors while importing AST -----*- C++ -*-===//
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
//  This file defines the ASTImportError class which basically defines the kind
//  of error while importing AST .
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTIMPORTERROR_H
#define LANGUAGE_CORE_AST_ASTIMPORTERROR_H

#include "toolchain/Support/Error.h"

namespace language::Core {

class ASTImportError : public toolchain::ErrorInfo<ASTImportError> {
public:
  /// \brief Kind of error when importing an AST component.
  enum ErrorKind {
    NameConflict,         /// Naming ambiguity (likely ODR violation).
    UnsupportedConstruct, /// Not supported node or case.
    Unknown               /// Other error.
  };

  ErrorKind Error;

  static char ID;

  ASTImportError() : Error(Unknown) {}
  ASTImportError(const ASTImportError &Other) : Error(Other.Error) {}
  ASTImportError &operator=(const ASTImportError &Other) {
    Error = Other.Error;
    return *this;
  }
  ASTImportError(ErrorKind Error) : Error(Error) {}

  std::string toString() const;

  void log(toolchain::raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_ASTIMPORTERROR_H
