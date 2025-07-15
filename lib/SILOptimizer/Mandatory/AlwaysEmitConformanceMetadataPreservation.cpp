//===--- AlwaysEmitConformanceMetadataPreservation.cpp -------------------===//
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
///
/// Some frameworks may rely on conformances to protocols they provide
/// to be present in binary product they are compiled into even if
/// such conformances are not otherwise referenced in user code.
/// Such conformances may then, for example, be queried and used by the
/// runtime.
///
/// The developer may not ever explicitly reference or instantiate this type in
/// their code, as it is effectively defining an XPC endpoint. However, when
/// optimizations are enabled, the type may be stripped from the binary as it is
/// never referenced. `@_alwaysEmitConformanceMetadata` can be used to mark
/// a protocol to ensure that its conformances are always marked as externally
/// visible even if not `public` to ensure they do not get optimized away.
/// This mandatory pass makes it so.
///
//===----------------------------------------------------------------------===//

#include "language/AST/Attr.h"
#define DEBUG_TYPE "always-emit-conformance-metadata-preservation"
#include "language/AST/ASTWalker.h"
#include "language/AST/NameLookup.h"
#include "language/AST/ProtocolConformance.h"
#include "language/AST/SourceFile.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

namespace {

/// A helper class to collect all nominal type declarations that conform to
/// `@_alwaysEmitConformanceMetadata` protocols.
class AlwaysEmitMetadataConformanceCollector : public ASTWalker {
  std::vector<NominalTypeDecl *> &AlwaysEmitMetadataConformanceDecls;

public:
  AlwaysEmitMetadataConformanceCollector(
      std::vector<NominalTypeDecl *> &AlwaysEmitMetadataConformanceDecls)
      : AlwaysEmitMetadataConformanceDecls(AlwaysEmitMetadataConformanceDecls) {
  }

  PreWalkAction walkToDeclPre(Decl *D) override {
    auto hasAlwaysEmitMetadataConformance =
        [&](toolchain::PointerUnion<const TypeDecl *, const ExtensionDecl *> Decl) {
          bool anyObject = false;
          InvertibleProtocolSet Inverses;
          for (const auto &found :
               getDirectlyInheritedNominalTypeDecls(Decl, Inverses, anyObject))
            if (auto Protocol = dyn_cast<ProtocolDecl>(found.Item))
              if (Protocol->getAttrs()
                      .hasAttribute<AlwaysEmitConformanceMetadataAttr>())
                return true;
          return false;
        };

    if (auto *NTD = dyn_cast<NominalTypeDecl>(D)) {
      if (hasAlwaysEmitMetadataConformance(NTD))
        AlwaysEmitMetadataConformanceDecls.push_back(NTD);
    } else if (auto *ETD = dyn_cast<ExtensionDecl>(D)) {
      if (hasAlwaysEmitMetadataConformance(ETD))
        AlwaysEmitMetadataConformanceDecls.push_back(ETD->getExtendedNominal());
    }

    // Visit peers expanded from macros
    D->visitAuxiliaryDecls([&](Decl *decl) { decl->walk(*this); },
                           /*visitFreestandingExpanded=*/false);

    return Action::Continue();
  }
};

class AlwaysEmitConformanceMetadataPreservation : public SILModuleTransform {
  void run() override {
    auto &M = *getModule();
    std::vector<NominalTypeDecl *> AlwaysEmitMetadataConformanceDecls;
    AlwaysEmitMetadataConformanceCollector Walker(
        AlwaysEmitMetadataConformanceDecls);

    SmallVector<Decl *> TopLevelDecls;
    if (M.getCodiraModule()->isMainModule()) {
      if (M.isWholeModule()) {
        for (const auto File : M.getCodiraModule()->getFiles())
          File->getTopLevelDecls(TopLevelDecls);
      } else {
        for (const auto Primary : M.getCodiraModule()->getPrimarySourceFiles()) {
          Primary->getTopLevelDecls(TopLevelDecls);
	  // Visit macro expanded extensions
	  if (auto *synthesizedPrimary = Primary->getSynthesizedFile())
	    synthesizedPrimary->getTopLevelDecls(TopLevelDecls);
	}
      }
    }
    for (auto *TLD : TopLevelDecls)
      TLD->walk(Walker);

    for (auto &NTD : AlwaysEmitMetadataConformanceDecls)
      M.addExternallyVisibleDecl(NTD);
  }
};
} // end anonymous namespace

SILTransform *language::createAlwaysEmitConformanceMetadataPreservation() {
  return new AlwaysEmitConformanceMetadataPreservation();
}
