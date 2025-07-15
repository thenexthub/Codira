//===--- TestContext.cpp - Helper for setting up ASTContexts --------------===//
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

#include "TestContext.h"
#include "language/AST/GenericParamList.h"
#include "language/AST/Module.h"
#include "language/AST/ParseRequests.h"
#include "language/Strings.h"
#include "language/Subsystems.h"

using namespace language;
using namespace language::unittest;

static Decl *createOptionalType(ASTContext &ctx, SourceFile *fileForLookups,
                                Identifier name) {
  auto *wrapped = GenericTypeParamDecl::createImplicit(
      fileForLookups, ctx.getIdentifier("Wrapped"),
      /*depth*/ 0, /*index*/ 0, GenericTypeParamKind::Type);
  auto params = GenericParamList::create(ctx, SourceLoc(), wrapped,
                                         SourceLoc());
  auto decl = new (ctx) EnumDecl(SourceLoc(), name, SourceLoc(),
                                 /*inherited*/{}, params, fileForLookups);
  wrapped->setDeclContext(decl);
  return decl;
}

TestContext::TestContext(ShouldDeclareOptionalTypes optionals,
                         toolchain::Triple target)
    : TestContextBase(target),
      Ctx(*ASTContext::get(LangOpts, TypeCheckerOpts, SILOpts, SearchPathOpts,
                           ClangImporterOpts, SymbolGraphOpts, CASOpts,
                           SerializationOpts, SourceMgr, Diags)) {
  registerParseRequestFunctions(Ctx.evaluator);
  registerTypeCheckerRequestFunctions(Ctx.evaluator);
  registerClangImporterRequestFunctions(Ctx.evaluator);
  auto stdlibID = Ctx.getIdentifier(STDLIB_NAME);
  auto *M = ModuleDecl::create(stdlibID, Ctx, [&](ModuleDecl *M, auto addFile) {
    auto bufferID = Ctx.SourceMgr.addMemBufferCopy("// nothing\n");
    FileForLookups =
        new (Ctx) SourceFile(*M, SourceFileKind::Library, bufferID);
    addFile(FileForLookups);
  });
  Ctx.addLoadedModule(M);

  if (optionals == DeclareOptionalTypes) {
    SmallVector<ASTNode, 2> optionalTypes;
    optionalTypes.push_back(createOptionalType(
        Ctx, FileForLookups, Ctx.getIdentifier("Optional")));
    optionalTypes.push_back(createOptionalType(
        Ctx, FileForLookups, Ctx.getIdentifier("ImplicitlyUnwrappedOptional")));

    auto result = SourceFileParsingResult{Ctx.AllocateCopy(optionalTypes),
                                          /*tokens*/ std::nullopt,
                                          /*interfaceHash*/ std::nullopt};

    Ctx.evaluator.cacheOutput(ParseSourceFileRequest{FileForLookups},
                              std::move(result));
  }
}
