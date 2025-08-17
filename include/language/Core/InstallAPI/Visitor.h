//===- InstallAPI/Visitor.h -----------------------------------*- C++ -*-===//
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
///
/// ASTVisitor Interface for InstallAPI frontend operations.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_INSTALLAPI_VISITOR_H
#define LANGUAGE_CORE_INSTALLAPI_VISITOR_H

#include "language/Core/AST/Mangle.h"
#include "language/Core/AST/RecursiveASTVisitor.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Frontend/FrontendActions.h"
#include "language/Core/InstallAPI/Context.h"
#include "toolchain/ADT/Twine.h"

namespace language::Core {
struct AvailabilityInfo;
namespace installapi {

/// ASTVisitor for collecting declarations that represent global symbols.
class InstallAPIVisitor final : public ASTConsumer,
                                public RecursiveASTVisitor<InstallAPIVisitor> {
public:
  InstallAPIVisitor(ASTContext &ASTCtx, InstallAPIContext &Ctx,
                    SourceManager &SrcMgr, Preprocessor &PP)
      : Ctx(Ctx), SrcMgr(SrcMgr), PP(PP),
        MC(ItaniumMangleContext::create(ASTCtx, ASTCtx.getDiagnostics())),
        Layout(ASTCtx.getTargetInfo().getDataLayoutString()) {}
  void HandleTranslationUnit(ASTContext &ASTCtx) override;
  bool shouldVisitTemplateInstantiations() const { return true; }

  /// Collect global variables.
  bool VisitVarDecl(const VarDecl *D);

  /// Collect global functions.
  bool VisitFunctionDecl(const FunctionDecl *D);

  /// Collect Objective-C Interface declarations.
  /// Every Objective-C class has an interface declaration that lists all the
  /// ivars, properties, and methods of the class.
  bool VisitObjCInterfaceDecl(const ObjCInterfaceDecl *D);

  /// Collect Objective-C Category/Extension declarations.
  ///
  /// The class that is being extended might come from a different library and
  /// is therefore itself not collected.
  bool VisitObjCCategoryDecl(const ObjCCategoryDecl *D);

  /// Collect global c++ declarations.
  bool VisitCXXRecordDecl(const CXXRecordDecl *D);

private:
  std::string getMangledName(const NamedDecl *D) const;
  std::string getBackendMangledName(toolchain::Twine Name) const;
  std::string getMangledCXXVTableName(const CXXRecordDecl *D) const;
  std::string getMangledCXXThunk(const GlobalDecl &D, const ThunkInfo &Thunk,
                                 bool ElideOverrideInfo) const;
  std::string getMangledCXXRTTI(const CXXRecordDecl *D) const;
  std::string getMangledCXXRTTIName(const CXXRecordDecl *D) const;
  std::string getMangledCtorDtor(const CXXMethodDecl *D, int Type) const;

  std::optional<HeaderType> getAccessForDecl(const NamedDecl *D) const;
  void recordObjCInstanceVariables(
      const ASTContext &ASTCtx, toolchain::MachO::ObjCContainerRecord *Record,
      StringRef SuperClass,
      const toolchain::iterator_range<
          DeclContext::specific_decl_iterator<ObjCIvarDecl>>
          Ivars);
  void emitVTableSymbols(const CXXRecordDecl *D, const AvailabilityInfo &Avail,
                         const HeaderType Access, bool EmittedVTable = false);

  InstallAPIContext &Ctx;
  SourceManager &SrcMgr;
  Preprocessor &PP;
  std::unique_ptr<language::Core::ItaniumMangleContext> MC;
  StringRef Layout;
};

} // namespace installapi
} // namespace language::Core

#endif // LANGUAGE_CORE_INSTALLAPI_VISITOR_H
