//===--- Edge.cpp - Symbol Graph Edge -------------------------------------===//
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

#include "language/AST/Module.h"
#include "Edge.h"
#include "Symbol.h"
#include "SymbolGraphASTWalker.h"

#include <queue>

using namespace language;
using namespace symbolgraphgen;

namespace {

bool parentDeclIsPrivate(const ValueDecl *VD, SymbolGraph *Graph) {
  if (!VD)
    return false;

  auto *ParentDecl = VD->getDeclContext()->getAsDecl();

  if (auto *ParentExtension = dyn_cast_or_null<ExtensionDecl>(ParentDecl)) {
    if (auto *Nominal = ParentExtension->getExtendedNominal()) {
      return Graph->isImplicitlyPrivate(Nominal);
    }
  }

  if (ParentDecl) {
    return Graph->isImplicitlyPrivate(ParentDecl);
  } else {
    return false;
  }
}

} // anonymous namespace

void Edge::serialize(toolchain::json::OStream &OS) const {
  OS.object([&](){
    OS.attribute("kind", Kind.Name);
    SmallString<256> SourceUSR, TargetUSR;

    Source.getUSR(SourceUSR);
    OS.attribute("source", SourceUSR.str());

    Target.getUSR(TargetUSR);
    OS.attribute("target", TargetUSR.str());

    // In case a dependent module isn't available, serialize a fallback name.
    auto TargetModuleName = Target.getSymbolDecl()
        ->getModuleContext()->getName().str();

    if (TargetModuleName != Graph->M.getName().str()) {
      SmallString<128> Scratch(TargetModuleName);
      toolchain::raw_svector_ostream PathOS(Scratch);
      PathOS << '.';
      Target.printPath(PathOS);
      OS.attribute("targetFallback", Scratch.str());
    }

    if (ConformanceExtension) {
      SmallVector<Requirement, 4> FilteredRequirements;
      filterGenericRequirements(
          ConformanceExtension->getGenericRequirements(),
          ConformanceExtension->getExtendedProtocolDecl(),
          FilteredRequirements);
      if (!FilteredRequirements.empty()) {
        OS.attributeArray("languageConstraints", [&](){
          for (const auto &Req : FilteredRequirements) {
            ::serialize(Req, OS);
          }
        });
      }
    }
    
    const ValueDecl *InheritingDecl = Source.getInheritedDecl();

    // If our source symbol is a inheriting decl, write in information about
    // where it's inheriting docs from.
    if (InheritingDecl && !parentDeclIsPrivate(InheritingDecl, Graph)) {
      Symbol inheritedSym(Graph, InheritingDecl, nullptr);
      SmallString<256> USR, Display;
      toolchain::raw_svector_ostream DisplayOS(Display);
      
      inheritedSym.getUSR(USR);
      inheritedSym.printPath(DisplayOS);
      
      OS.attributeObject("sourceOrigin", [&](){
        OS.attribute("identifier", USR.str());
        OS.attribute("displayName", Display.str());
      });
    }
  });
}
