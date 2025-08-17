//===----- CXXABI.h - Interface to C++ ABIs ---------------------*- C++ -*-===//
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
// This provides an abstract class for C++ AST support. Concrete
// subclasses of this implement AST support for specific C++ ABIs.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_AST_CXXABI_H
#define LANGUAGE_CORE_LIB_AST_CXXABI_H

#include "language/Core/AST/Type.h"

namespace language::Core {

class ASTContext;
class CXXConstructorDecl;
class DeclaratorDecl;
class MangleContext;
class MangleNumberingContext;
class MemberPointerType;

/// Implements C++ ABI-specific semantic analysis functions.
class CXXABI {
public:
  virtual ~CXXABI();

  struct MemberPointerInfo {
    uint64_t Width;
    unsigned Align;
    bool HasPadding;
  };

  /// Returns the width and alignment of a member pointer in bits, as well as
  /// whether it has padding.
  virtual MemberPointerInfo
  getMemberPointerInfo(const MemberPointerType *MPT) const = 0;

  /// Returns the default calling convention for C++ methods.
  virtual CallingConv getDefaultMethodCallConv(bool isVariadic) const = 0;

  /// Returns whether the given class is nearly empty, with just virtual
  /// pointers and no data except possibly virtual bases.
  virtual bool isNearlyEmpty(const CXXRecordDecl *RD) const = 0;

  /// Returns a new mangling number context for this C++ ABI.
  virtual std::unique_ptr<MangleNumberingContext>
  createMangleNumberingContext() const = 0;

  /// Adds a mapping from class to copy constructor for this C++ ABI.
  virtual void addCopyConstructorForExceptionObject(CXXRecordDecl *,
                                                    CXXConstructorDecl *) = 0;

  /// Retrieves the mapping from class to copy constructor for this C++ ABI.
  virtual const CXXConstructorDecl *
  getCopyConstructorForExceptionObject(CXXRecordDecl *) = 0;

  virtual void addTypedefNameForUnnamedTagDecl(TagDecl *TD,
                                               TypedefNameDecl *DD) = 0;

  virtual TypedefNameDecl *
  getTypedefNameForUnnamedTagDecl(const TagDecl *TD) = 0;

  virtual void addDeclaratorForUnnamedTagDecl(TagDecl *TD,
                                              DeclaratorDecl *DD) = 0;

  virtual DeclaratorDecl *getDeclaratorForUnnamedTagDecl(const TagDecl *TD) = 0;
};

/// Creates an instance of a C++ ABI class.
CXXABI *CreateItaniumCXXABI(ASTContext &Ctx);
CXXABI *CreateMicrosoftCXXABI(ASTContext &Ctx);
std::unique_ptr<MangleNumberingContext>
createItaniumNumberingContext(MangleContext *);
}

#endif
