//===--- SILGenRequests.cpp - Requests for SIL Generation  ----------------===//
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

#include "language/AST/SILGenRequests.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Module.h"
#include "language/AST/FileUnit.h"
#include "language/AST/SourceFile.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILModule.h"
#include "language/Subsystems.h"

using namespace language;

namespace language {
// Implement the SILGen type zone (zone 12).
#define LANGUAGE_TYPEID_ZONE SILGen
#define LANGUAGE_TYPEID_HEADER "language/AST/SILGenTypeIDZone.def"
#include "language/Basic/ImplementTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER
} // end namespace language

void language::simple_display(toolchain::raw_ostream &out,
                           const ASTLoweringDescriptor &desc) {
  auto *MD = desc.context.dyn_cast<ModuleDecl *>();
  auto *unit = desc.context.dyn_cast<FileUnit *>();
  if (MD) {
    out << "Lowering AST to SIL for module " << MD->getName();
  } else {
    assert(unit);
    out << "Lowering AST to SIL for file ";
    simple_display(out, unit);
  }
}

SourceLoc language::extractNearestSourceLoc(const ASTLoweringDescriptor &desc) {
  return SourceLoc();
}

evaluator::DependencySource ASTLoweringRequest::readDependencySource(
    const evaluator::DependencyRecorder &e) const {
  auto &desc = std::get<0>(getStorage());

  // We don't track dependencies in whole-module mode.
  if (desc.context.is<ModuleDecl *>()) {
    return nullptr;
  }

  // If we have a single source file, it's the source of dependencies.
  return dyn_cast<SourceFile>(desc.context.get<FileUnit *>());
}

ArrayRef<FileUnit *> ASTLoweringDescriptor::getFilesToEmit() const {
  // If we have a specific set of SILDeclRefs to emit, we don't emit any whole
  // files.
  if (SourcesToEmit)
    return {};

  if (auto *mod = context.dyn_cast<ModuleDecl *>())
    return mod->getFiles();

  // For a single file, we can form an ArrayRef that points at its storage in
  // the union.
  return toolchain::ArrayRef(*context.getAddrOfPtr1());
}

SourceFile *ASTLoweringDescriptor::getSourceFileToParse() const {
#ifndef NDEBUG
  auto sfCount = toolchain::count_if(getFilesToEmit(), [](FileUnit *file) {
    return isa<SourceFile>(file);
  });
  auto silFileCount = toolchain::count_if(getFilesToEmit(), [](FileUnit *file) {
    auto *SF = dyn_cast<SourceFile>(file);
    return SF && SF->Kind == SourceFileKind::SIL;
  });
  assert(silFileCount == 0 || (silFileCount == 1 && sfCount == 1) &&
         "Cannot currently mix a .sil file with other SourceFiles");
#endif

  for (auto *file : getFilesToEmit()) {
    // Skip other kinds of files.
    auto *SF = dyn_cast<SourceFile>(file);
    if (!SF)
      continue;

    // Given the above precondition that a .sil file isn't mixed with other
    // SourceFiles, we can return a SIL file if we have it, or return nullptr.
    if (SF->Kind == SourceFileKind::SIL) {
      return SF;
    } else {
      return nullptr;
    }
  }
  return nullptr;
}

// Define request evaluation functions for each of the SILGen requests.
static AbstractRequestFunction *silGenRequestFunctions[] = {
#define LANGUAGE_REQUEST(Zone, Name, Sig, Caching, LocOptions)                    \
  reinterpret_cast<AbstractRequestFunction *>(&Name::evaluateRequest),
#include "language/AST/SILGenTypeIDZone.def"
#undef LANGUAGE_REQUEST
};

void language::registerSILGenRequestFunctions(Evaluator &evaluator) {
  evaluator.registerRequestFunctions(Zone::SILGen,
                                     silGenRequestFunctions);
}
