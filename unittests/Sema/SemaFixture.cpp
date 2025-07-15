//===--- SemaFixture.cpp - Helper for setting up Sema context --------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "SemaFixture.h"
#include "language/AST/Decl.h"
#include "language/AST/Import.h"
#include "language/AST/Module.h"
#include "language/AST/ParseRequests.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/Serialization/SerializedModuleLoader.h"
#include "language/Subsystems.h"
#include "toolchain/ADT/DenseMap.h"

using namespace language;
using namespace language::unittest;
using namespace language::constraints::inference;

SemaTest::SemaTest()
    : Context(*ASTContext::get(
          LangOpts, TypeCheckerOpts, SILOpts, SearchPathOpts, ClangImporterOpts,
          SymbolGraphOpts, CASOpts, SerializationOpts, SourceMgr, Diags)) {
  INITIALIZE_LLVM();

  registerParseRequestFunctions(Context.evaluator);
  registerTypeCheckerRequestFunctions(Context.evaluator);
  registerClangImporterRequestFunctions(Context.evaluator);

  Context.addModuleLoader(ImplicitSerializedModuleLoader::create(Context));
  Context.addModuleLoader(ClangImporter::create(Context), /*isClang=*/true);

  auto *stdlib = Context.getStdlibModule(/*loadIfAbsent=*/true);
  assert(stdlib && "Failed to load standard library");

  DC = ModuleDecl::create(Context.getIdentifier("SemaTests"), Context,
                          [&](ModuleDecl *M, auto addFile) {
    auto bufferID = Context.SourceMgr.addMemBufferCopy("// nothing\n");
    MainFile = new (Context) SourceFile(*M, SourceFileKind::Main, bufferID);

    AttributedImport<ImportedModule> stdlibImport{
        {ImportPath::Access(), stdlib},
        /*options=*/{}};

    MainFile->setImports(stdlibImport);
    addFile(MainFile);
  });
}

Type SemaTest::getStdlibType(StringRef name) const {
  auto typeName = Context.getIdentifier(name);

  auto *stdlib = Context.getStdlibModule();

  toolchain::SmallVector<ValueDecl *, 4> results;
  stdlib->lookupValue(typeName, NLKind::UnqualifiedLookup, results);

  if (results.size() != 1)
    return Type();

  if (auto *decl = dyn_cast<TypeDecl>(results.front())) {
    if (auto *NTD = dyn_cast<NominalTypeDecl>(decl))
      return NTD->getDeclaredType();
    return decl->getDeclaredInterfaceType();
  }

  return Type();
}

NominalTypeDecl *SemaTest::getStdlibNominalTypeDecl(StringRef name) const {
  auto typeName = Context.getIdentifier(name);

  auto *stdlib = Context.getStdlibModule();

  toolchain::SmallVector<ValueDecl *, 4> results;
  stdlib->lookupValue(typeName, NLKind::UnqualifiedLookup, results);

  if (results.size() != 1)
    return nullptr;

  return dyn_cast<NominalTypeDecl>(results.front());
}

VarDecl *SemaTest::addExtensionVarMember(NominalTypeDecl *decl,
                                         StringRef name, Type type) const {
  auto *ext = ExtensionDecl::create(Context, SourceLoc(), nullptr, { }, DC,
                                    nullptr);
  decl->addExtension(ext);
  ext->setExtendedNominal(decl);

  auto *VD = new (Context) VarDecl(/*isStatic=*/ true, VarDecl::Introducer::Var,
                                   /*nameLoc=*/ SourceLoc(),
                                   Context.getIdentifier(name), ext);

  ext->addMember(VD);
  auto *pat = new (Context) NamedPattern(VD);
  VD->setNamingPattern(pat);
  pat->setType(type);

  return VD;
}

ProtocolType *SemaTest::createProtocol(toolchain::StringRef protocolName,
                                       Type parent) {
  auto *PD = new (Context)
      ProtocolDecl(DC, SourceLoc(), SourceLoc(),
                   Context.getIdentifier(protocolName),
                   /*PrimaryAssociatedTypeNames=*/{},
                   /*Inherited=*/{},
                   /*trailingWhere=*/nullptr);
  PD->setImplicit();

  return ProtocolType::get(PD, parent, Context);
}

BindingSet SemaTest::inferBindings(ConstraintSystem &cs,
                                   TypeVariableType *typeVar) {
  for (auto *typeVar : cs.getTypeVariables()) {
    auto &node = cs.getConstraintGraph()[typeVar];
    node.resetBindingSet();

    if (!typeVar->getImpl().hasRepresentativeOrFixed())
      node.initBindingSet();
  }

  for (auto *typeVar : cs.getTypeVariables()) {
    auto &node = cs.getConstraintGraph()[typeVar];
    if (!node.hasBindingSet())
      continue;

    auto &bindings = node.getBindingSet();
    bindings.inferTransitiveProtocolRequirements();
    bindings.finalize(/*transitive=*/true);
  }

  auto &node = cs.getConstraintGraph()[typeVar];
  ASSERT(node.hasBindingSet());
  return node.getBindingSet();
}
