//===--- ClangImporterRequests.cpp - Clang Importer Requests --------------------===//
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

#include "language/AST/NameLookupRequests.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/Evaluator.h"
#include "language/AST/Module.h"
#include "language/AST/SourceFile.h"
#include "language/ClangImporter/ClangImporterRequests.h"
#include "language/Subsystems.h"
#include "ImporterImpl.h"

using namespace language;

std::optional<ObjCInterfaceAndImplementation>
ObjCInterfaceAndImplementationRequest::getCachedResult() const {
  auto passedDecl = std::get<0>(getStorage());
  if (!passedDecl) {
    return {};
  }

  if (passedDecl->getCachedLacksObjCInterfaceOrImplementation()) {
    // We've computed this request and found that this is a normal declaration.
    return {};
  }

  // Either we've computed this request and cached a result in the ImporterImpl,
  // or we haven't computed this request. Check the caches.
  auto &ctx = passedDecl->getASTContext();
  auto importer = static_cast<ClangImporter *>(ctx.getClangModuleLoader());
  auto &impl = importer->Impl;

  // We need the full list of interfaces for a given implementation, but that's
  // only available through `InterfacesByImplementation`. So we must first
  // figure out the key into that cache, which might require a reverse lookup in
  // `ImplementationsByInterface`.

  Decl *implDecl = nullptr;
  if (passedDecl->hasClangNode()) {
    // `passedDecl` *could* be an interface.
    auto iter = impl.ImplementationsByInterface.find(passedDecl);
    if (iter != impl.ImplementationsByInterface.end())
      implDecl = iter->second;
  } else {
    // `passedDecl` *could* be an implementation.
    implDecl = passedDecl;
  }

  if (implDecl) {
    auto iter = impl.InterfacesByImplementation.find(implDecl);
    if (iter != impl.InterfacesByImplementation.end()) {
      return ObjCInterfaceAndImplementation(iter->second, implDecl);
    }
  }

  // Nothing in the caches, so we must need to compute this.
  return std::nullopt;
}

void ObjCInterfaceAndImplementationRequest::
cacheResult(ObjCInterfaceAndImplementation value) const {
  Decl *passedDecl = std::get<0>(getStorage());

  if (value.empty()) {
    // `decl` is neither an interface nor an implementation; remember this.
    passedDecl->setCachedLacksObjCInterfaceOrImplementation(true);
    return;
  }

  // Cache computed pointers from implementations to interfaces.
  auto &ctx = passedDecl->getASTContext();
  auto importer = static_cast<ClangImporter *>(ctx.getClangModuleLoader());
  auto &impl = importer->Impl;

  impl.InterfacesByImplementation.insert({ value.implementationDecl,
                                           value.interfaceDecls });
  for (auto interfaceDecl : value.interfaceDecls)
    impl.ImplementationsByInterface.insert({ interfaceDecl,
                                             value.implementationDecl });

  // If this was a duplicate implementation, cache a null so we don't recompute.
  if (!passedDecl->hasClangNode() && passedDecl != value.implementationDecl) {
    passedDecl->setCachedLacksObjCInterfaceOrImplementation(true);
  }
}

// Define request evaluation functions for each of the name lookup requests.
static AbstractRequestFunction *clangImporterRequestFunctions[] = {
#define LANGUAGE_REQUEST(Zone, Name, Sig, Caching, LocOptions)                    \
  reinterpret_cast<AbstractRequestFunction *>(&Name::evaluateRequest),
#include "language/ClangImporter/ClangImporterTypeIDZone.def"
#undef LANGUAGE_REQUEST
};

void language::registerClangImporterRequestFunctions(Evaluator &evaluator) {
  evaluator.registerRequestFunctions(Zone::ClangImporter,
                                     clangImporterRequestFunctions);
}

