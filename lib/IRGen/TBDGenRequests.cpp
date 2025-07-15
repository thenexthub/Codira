//===--- TBDGenRequests.cpp - Requests for TBD Generation  ----------------===//
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

#include "language/AST/TBDGenRequests.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Evaluator.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Module.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/IRGen/TBDGen.h"
#include "language/Subsystems.h"
#include "clang/Basic/TargetInfo.h"
#include "toolchain/TextAPI/InterfaceFile.h"

#include "APIGen.h"

using namespace language;

namespace language {
// Implement the TBDGen type zone (zone 14).
#define LANGUAGE_TYPEID_ZONE TBDGen
#define LANGUAGE_TYPEID_HEADER "language/AST/TBDGenTypeIDZone.def"
#include "language/Basic/ImplementTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER
} // end namespace language

//----------------------------------------------------------------------------//
// GenerateTBDRequest computation.
//----------------------------------------------------------------------------//

FileUnit *TBDGenDescriptor::getSingleFile() const {
  return Input.dyn_cast<FileUnit *>();
}

ModuleDecl *TBDGenDescriptor::getParentModule() const {
  if (auto *module = Input.dyn_cast<ModuleDecl *>())
    return module;
  return Input.get<FileUnit *>()->getParentModule();
}

const StringRef TBDGenDescriptor::getDataLayoutString() const {
  auto &ctx = getParentModule()->getASTContext();
  auto *clang = static_cast<ClangImporter *>(ctx.getClangModuleLoader());
  return toolchain::StringRef(clang->getTargetInfo().getDataLayoutString());
}

const toolchain::Triple &TBDGenDescriptor::getTarget() const {
  return getParentModule()->getASTContext().LangOpts.Target;
}

bool TBDGenDescriptor::operator==(const TBDGenDescriptor &other) const {
  return Input == other.Input && Opts == other.Opts;
}

toolchain::hash_code language::hash_value(const TBDGenDescriptor &desc) {
  return toolchain::hash_combine(desc.getFileOrModule(), desc.getOptions());
}

void language::simple_display(toolchain::raw_ostream &out,
                           const TBDGenDescriptor &desc) {
  out << "Generate TBD for ";
  if (auto *module = desc.getFileOrModule().dyn_cast<ModuleDecl *>()) {
    out << "module ";
    simple_display(out, module);
  } else {
    out << "file ";
    simple_display(out, desc.getFileOrModule().get<FileUnit *>());
  }
}

SourceLoc language::extractNearestSourceLoc(const TBDGenDescriptor &desc) {
  return extractNearestSourceLoc(desc.getFileOrModule());
}

// Define request evaluation functions for each of the TBDGen requests.
static AbstractRequestFunction *tbdGenRequestFunctions[] = {
#define LANGUAGE_REQUEST(Zone, Name, Sig, Caching, LocOptions)                    \
  reinterpret_cast<AbstractRequestFunction *>(&Name::evaluateRequest),
#include "language/AST/TBDGenTypeIDZone.def"
#undef LANGUAGE_REQUEST
};

void language::registerTBDGenRequestFunctions(Evaluator &evaluator) {
  evaluator.registerRequestFunctions(Zone::TBDGen, tbdGenRequestFunctions);
}
