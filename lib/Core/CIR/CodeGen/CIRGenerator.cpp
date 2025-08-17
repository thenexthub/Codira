//===--- CIRGenerator.cpp - Emit CIR from ASTs ----------------------------===//
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
// This builds an AST and converts it to CIR.
//
//===----------------------------------------------------------------------===//

#include "CIRGenModule.h"

#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Target/LLVMIR/Import.h"

#include "language/Core/AST/DeclGroup.h"
#include "language/Core/CIR/CIRGenerator.h"
#include "language/Core/CIR/Dialect/IR/CIRDialect.h"
#include "language/Core/CIR/Dialect/OpenACC/RegisterOpenACCExtensions.h"
#include "toolchain/IR/DataLayout.h"

using namespace cir;
using namespace language::Core;

void CIRGenerator::anchor() {}

CIRGenerator::CIRGenerator(language::Core::DiagnosticsEngine &diags,
                           toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> vfs,
                           const CodeGenOptions &cgo)
    : diags(diags), fs(std::move(vfs)), codeGenOpts{cgo},
      handlingTopLevelDecls{0} {}
CIRGenerator::~CIRGenerator() {
  // There should normally not be any leftover inline method definitions.
  assert(deferredInlineMemberFuncDefs.empty() || diags.hasErrorOccurred());
}

static void setMLIRDataLayout(mlir::ModuleOp &mod, const toolchain::DataLayout &dl) {
  mlir::MLIRContext *mlirContext = mod.getContext();
  mlir::DataLayoutSpecInterface dlSpec =
      mlir::translateDataLayout(dl, mlirContext);
  mod->setAttr(mlir::DLTIDialect::kDataLayoutAttrName, dlSpec);
}

void CIRGenerator::Initialize(ASTContext &astContext) {
  using namespace toolchain;

  this->astContext = &astContext;

  mlirContext = std::make_unique<mlir::MLIRContext>();
  mlirContext->loadDialect<mlir::DLTIDialect>();
  mlirContext->loadDialect<cir::CIRDialect>();
  mlirContext->getOrLoadDialect<mlir::acc::OpenACCDialect>();

  // Register extensions to integrate CIR types with OpenACC.
  mlir::DialectRegistry registry;
  cir::acc::registerOpenACCExtensions(registry);
  mlirContext->appendDialectRegistry(registry);

  cgm = std::make_unique<language::Core::CIRGen::CIRGenModule>(
      *mlirContext.get(), astContext, codeGenOpts, diags);
  mlir::ModuleOp mod = cgm->getModule();
  toolchain::DataLayout layout =
      toolchain::DataLayout(astContext.getTargetInfo().getDataLayoutString());
  setMLIRDataLayout(mod, layout);
}

bool CIRGenerator::verifyModule() const { return cgm->verifyModule(); }

mlir::ModuleOp CIRGenerator::getModule() const { return cgm->getModule(); }

bool CIRGenerator::HandleTopLevelDecl(DeclGroupRef group) {
  if (diags.hasUnrecoverableErrorOccurred())
    return true;

  HandlingTopLevelDeclRAII handlingDecl(*this);

  for (Decl *decl : group)
    cgm->emitTopLevelDecl(decl);

  return true;
}

void CIRGenerator::HandleTranslationUnit(ASTContext &astContext) {
  // Release the Builder when there is no error.
  if (!diags.hasErrorOccurred() && cgm)
    cgm->release();

  // If there are errors before or when releasing the cgm, reset the module to
  // stop here before invoking the backend.
  assert(!cir::MissingFeatures::cleanupAfterErrorDiags());
}

void CIRGenerator::HandleInlineFunctionDefinition(FunctionDecl *d) {
  if (diags.hasErrorOccurred())
    return;

  assert(d->doesThisDeclarationHaveABody());

  // We may want to emit this definition. However, that decision might be
  // based on computing the linkage, and we have to defer that in case we are
  // inside of something that will chagne the method's final linkage, e.g.
  //   typedef struct {
  //     void bar();
  //     void foo() { bar(); }
  //   } A;
  deferredInlineMemberFuncDefs.push_back(d);

  // Provide some coverage mapping even for methods that aren't emitted.
  // Don't do this for templated classes though, as they may not be
  // instantiable.
  assert(!cir::MissingFeatures::coverageMapping());
}

void CIRGenerator::emitDeferredDecls() {
  if (deferredInlineMemberFuncDefs.empty())
    return;

  // Emit any deferred inline method definitions. Note that more deferred
  // methods may be added during this loop, since ASTConsumer callbacks can be
  // invoked if AST inspection results in declarations being added. Therefore,
  // we use an index to loop over the deferredInlineMemberFuncDefs rather than
  // a range.
  HandlingTopLevelDeclRAII handlingDecls(*this);
  for (unsigned i = 0; i != deferredInlineMemberFuncDefs.size(); ++i)
    cgm->emitTopLevelDecl(deferredInlineMemberFuncDefs[i]);
  deferredInlineMemberFuncDefs.clear();
}

/// HandleTagDeclDefinition - This callback is invoked each time a TagDecl to
/// (e.g. struct, union, enum, class) is completed. This allows the client to
/// hack on the type, which can occur at any point in the file (because these
/// can be defined in declspecs).
void CIRGenerator::HandleTagDeclDefinition(TagDecl *d) {
  if (diags.hasErrorOccurred())
    return;

  // Don't allow re-entrant calls to CIRGen triggered by PCH deserialization to
  // emit deferred decls.
  HandlingTopLevelDeclRAII handlingDecl(*this, /*EmitDeferred=*/false);

  cgm->updateCompletedType(d);

  // For MSVC compatibility, treat declarations of static data members with
  // inline initializers as definitions.
  if (astContext->getTargetInfo().getCXXABI().isMicrosoft())
    cgm->errorNYI(d->getSourceRange(), "HandleTagDeclDefinition: MSABI");
  // For OpenMP emit declare reduction functions, if required.
  if (astContext->getLangOpts().OpenMP)
    cgm->errorNYI(d->getSourceRange(), "HandleTagDeclDefinition: OpenMP");
}

void CIRGenerator::HandleTagDeclRequiredDefinition(const TagDecl *D) {
  if (diags.hasErrorOccurred())
    return;

  assert(!cir::MissingFeatures::generateDebugInfo());
}

void CIRGenerator::HandleCXXStaticMemberVarInstantiation(VarDecl *D) {
  if (diags.hasErrorOccurred())
    return;

  cgm->handleCXXStaticMemberVarInstantiation(D);
}

void CIRGenerator::CompleteTentativeDefinition(VarDecl *d) {
  if (diags.hasErrorOccurred())
    return;

  cgm->emitTentativeDefinition(d);
}

void CIRGenerator::HandleVTable(CXXRecordDecl *rd) {
  if (diags.hasErrorOccurred())
    return;

  cgm->errorNYI(rd->getSourceRange(), "HandleVTable");
}
