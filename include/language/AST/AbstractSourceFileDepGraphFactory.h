//===----- AbstractSourceFileDepGraphFactory.h ----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_SOURCE_FILE_DEP_GRAPH_CONSTRUCTOR_H
#define LANGUAGE_AST_SOURCE_FILE_DEP_GRAPH_CONSTRUCTOR_H

#include "language/AST/Decl.h"
#include "language/AST/DeclContext.h"
#include "language/AST/FineGrainedDependencies.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include <optional>

namespace language {
class DiagnosticEngine;
namespace fine_grained_dependencies {

/// Abstract class for building a \c SourceFileDepGraph from either a real
/// \c SourceFile or a unit test
class AbstractSourceFileDepGraphFactory {
protected:
  /// If there was an error, cannot get accurate info.
  const bool hadCompilationError;

  /// The name of the languageDeps file.
  const std::string languageDeps;

  /// The fingerprint of the whole file
  Fingerprint fileFingerprint;

  /// For debugging
  const bool emitDotFileAfterConstruction;

  DiagnosticEngine &diags;

  /// OutputBackend.
  toolchain::vfs::OutputBackend &backend;

  /// Graph under construction
  SourceFileDepGraph g;

public:
  /// Expose this layer to enable faking up a constructor for testing.
  /// See the instance variable comments for explanation.
  AbstractSourceFileDepGraphFactory(bool hadCompilationError,
                                    StringRef languageDeps,
                                    Fingerprint fileFingerprint,
                                    bool emitDotFileAfterConstruction,
                                    DiagnosticEngine &diags,
                                    toolchain::vfs::OutputBackend &outputBackend);

  virtual ~AbstractSourceFileDepGraphFactory() = default;

  /// Create a SourceFileDepGraph.
  SourceFileDepGraph construct();

private:
  void addSourceFileNodesToGraph();

  /// Add the "provides" nodes when mocking up a graph
  virtual void addAllDefinedDecls() = 0;

  /// Add the "depends" nodes and arcs when mocking a graph
  virtual void addAllUsedDecls() = 0;

protected:
  /// Given an array of Decls or pairs of them in \p declsOrPairs
  /// create node pairs for context and name
  template <NodeKind kind, typename ContentsT>
  void addAllDefinedDeclsOfAGivenType(std::vector<ContentsT> &contentsVec) {
    for (const auto &declOrPair : contentsVec) {
      auto fp =
          AbstractSourceFileDepGraphFactory::getFingerprintIfAny(declOrPair);
      auto key = DependencyKey::Builder{kind, DeclAspect::interface}
                    .withContext(declOrPair)
                    .withName(declOrPair)
                    .build();
      addADefinedDecl(key, fp);
    }
  }

  /// Add an pair of interface, implementation nodes to the graph, which
  /// represent some \c Decl defined in this source file. \param key the
  /// interface key of the pair
  void addADefinedDecl(const DependencyKey &key,
                       std::optional<Fingerprint> fingerprint);

  void addAUsedDecl(const DependencyKey &def, const DependencyKey &use);

  /// Add an external dependency node to the graph. If the provided fingerprint
  /// is not \c None, it is added to the def key.
  void
  addAnExternalDependency(const DependencyKey &def, const DependencyKey &use,
                          std::optional<Fingerprint> dependencyFingerprint);

  static std::optional<Fingerprint>
  getFingerprintIfAny(std::pair<const NominalTypeDecl *, const ValueDecl *>) {
    return std::nullopt;
  }

  static std::optional<Fingerprint> getFingerprintIfAny(const Decl *d) {
    if (const auto *idc = dyn_cast<IterableDeclContext>(d)) {
      return idc->getBodyFingerprint();
    }
    return std::nullopt;
  }
};

} // namespace fine_grained_dependencies
} // namespace language

#endif // LANGUAGE_AST_SOURCE_FILE_DEP_GRAPH_CONSTRUCTOR_H
