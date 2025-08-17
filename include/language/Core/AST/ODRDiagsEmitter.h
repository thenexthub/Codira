//===- ODRDiagsEmitter.h - Emits diagnostic for ODR mismatches --*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_ODRDIAGSEMITTER_H
#define LANGUAGE_CORE_AST_ODRDIAGSEMITTER_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/LangOptions.h"

namespace language::Core {

class ODRDiagsEmitter {
public:
  ODRDiagsEmitter(DiagnosticsEngine &Diags, const ASTContext &Context,
                  const LangOptions &LangOpts)
      : Diags(Diags), Context(Context), LangOpts(LangOpts) {}

  /// Diagnose ODR mismatch between 2 FunctionDecl.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool diagnoseMismatch(const FunctionDecl *FirstFunction,
                        const FunctionDecl *SecondFunction) const;

  /// Diagnose ODR mismatch between 2 EnumDecl.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool diagnoseMismatch(const EnumDecl *FirstEnum,
                        const EnumDecl *SecondEnum) const;

  /// Diagnose ODR mismatch between 2 CXXRecordDecl.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  /// To compare 2 declarations with merged and identical definition data
  /// you need to provide pre-merge definition data in \p SecondDD.
  bool
  diagnoseMismatch(const CXXRecordDecl *FirstRecord,
                   const CXXRecordDecl *SecondRecord,
                   const struct CXXRecordDecl::DefinitionData *SecondDD) const;

  /// Diagnose ODR mismatch between 2 RecordDecl that are not CXXRecordDecl.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool diagnoseMismatch(const RecordDecl *FirstRecord,
                        const RecordDecl *SecondRecord) const;

  /// Diagnose ODR mismatch between 2 ObjCInterfaceDecl.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool diagnoseMismatch(
      const ObjCInterfaceDecl *FirstID, const ObjCInterfaceDecl *SecondID,
      const struct ObjCInterfaceDecl::DefinitionData *SecondDD) const;

  /// Diagnose ODR mismatch between ObjCInterfaceDecl with different
  /// definitions.
  bool diagnoseMismatch(const ObjCInterfaceDecl *FirstID,
                        const ObjCInterfaceDecl *SecondID) const {
    assert(FirstID->data().Definition != SecondID->data().Definition &&
           "Don't diagnose differences when definitions are merged already");
    return diagnoseMismatch(FirstID, SecondID, &SecondID->data());
  }

  /// Diagnose ODR mismatch between 2 ObjCProtocolDecl.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  /// To compare 2 declarations with merged and identical definition data
  /// you need to provide pre-merge definition data in \p SecondDD.
  bool diagnoseMismatch(
      const ObjCProtocolDecl *FirstProtocol,
      const ObjCProtocolDecl *SecondProtocol,
      const struct ObjCProtocolDecl::DefinitionData *SecondDD) const;

  /// Diagnose ODR mismatch between ObjCProtocolDecl with different definitions.
  bool diagnoseMismatch(const ObjCProtocolDecl *FirstProtocol,
                        const ObjCProtocolDecl *SecondProtocol) const {
    assert(FirstProtocol->data().Definition !=
               SecondProtocol->data().Definition &&
           "Don't diagnose differences when definitions are merged already");
    return diagnoseMismatch(FirstProtocol, SecondProtocol,
                            &SecondProtocol->data());
  }

  /// Get the best name we know for the module that owns the given
  /// declaration, or an empty string if the declaration is not from a module.
  static std::string getOwningModuleNameForDiagnostic(const Decl *D);

private:
  using DeclHashes = toolchain::SmallVector<std::pair<const Decl *, unsigned>, 4>;

  // Used with err_module_odr_violation_mismatch_decl,
  // note_module_odr_violation_mismatch_decl,
  // err_module_odr_violation_mismatch_decl_unknown,
  // and note_module_odr_violation_mismatch_decl_unknown
  // This list should be the same Decl's as in ODRHash::isSubDeclToBeProcessed
  enum ODRMismatchDecl {
    EndOfClass,
    PublicSpecifer,
    PrivateSpecifer,
    ProtectedSpecifer,
    StaticAssert,
    Field,
    CXXMethod,
    TypeAlias,
    TypeDef,
    Var,
    Friend,
    FunctionTemplate,
    ObjCMethod,
    ObjCIvar,
    ObjCProperty,
    Other
  };

  struct DiffResult {
    const Decl *FirstDecl = nullptr, *SecondDecl = nullptr;
    ODRMismatchDecl FirstDiffType = Other, SecondDiffType = Other;
  };

  // If there is a diagnoseable difference, FirstDiffType and
  // SecondDiffType will not be Other and FirstDecl and SecondDecl will be
  // filled in if not EndOfClass.
  static DiffResult FindTypeDiffs(DeclHashes &FirstHashes,
                                  DeclHashes &SecondHashes);

  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags.Report(Loc, DiagID);
  }

  // Use this to diagnose that an unexpected Decl was encountered
  // or no difference was detected. This causes a generic error
  // message to be emitted.
  void diagnoseSubMismatchUnexpected(DiffResult &DR,
                                     const NamedDecl *FirstRecord,
                                     StringRef FirstModule,
                                     const NamedDecl *SecondRecord,
                                     StringRef SecondModule) const;

  void diagnoseSubMismatchDifferentDeclKinds(DiffResult &DR,
                                             const NamedDecl *FirstRecord,
                                             StringRef FirstModule,
                                             const NamedDecl *SecondRecord,
                                             StringRef SecondModule) const;

  bool diagnoseSubMismatchField(const NamedDecl *FirstRecord,
                                StringRef FirstModule, StringRef SecondModule,
                                const FieldDecl *FirstField,
                                const FieldDecl *SecondField) const;

  bool diagnoseSubMismatchTypedef(const NamedDecl *FirstRecord,
                                  StringRef FirstModule, StringRef SecondModule,
                                  const TypedefNameDecl *FirstTD,
                                  const TypedefNameDecl *SecondTD,
                                  bool IsTypeAlias) const;

  bool diagnoseSubMismatchVar(const NamedDecl *FirstRecord,
                              StringRef FirstModule, StringRef SecondModule,
                              const VarDecl *FirstVD,
                              const VarDecl *SecondVD) const;

  /// Check if protocol lists are the same and diagnose if they are different.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool diagnoseSubMismatchProtocols(const ObjCProtocolList &FirstProtocols,
                                    const ObjCContainerDecl *FirstContainer,
                                    StringRef FirstModule,
                                    const ObjCProtocolList &SecondProtocols,
                                    const ObjCContainerDecl *SecondContainer,
                                    StringRef SecondModule) const;

  /// Check if Objective-C methods are the same and diagnose if different.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool diagnoseSubMismatchObjCMethod(const NamedDecl *FirstObjCContainer,
                                     StringRef FirstModule,
                                     StringRef SecondModule,
                                     const ObjCMethodDecl *FirstMethod,
                                     const ObjCMethodDecl *SecondMethod) const;

  /// Check if Objective-C properties are the same and diagnose if different.
  ///
  /// Returns true if found a mismatch and diagnosed it.
  bool
  diagnoseSubMismatchObjCProperty(const NamedDecl *FirstObjCContainer,
                                  StringRef FirstModule, StringRef SecondModule,
                                  const ObjCPropertyDecl *FirstProp,
                                  const ObjCPropertyDecl *SecondProp) const;

private:
  DiagnosticsEngine &Diags;
  const ASTContext &Context;
  const LangOptions &LangOpts;
};

} // namespace language::Core

#endif
