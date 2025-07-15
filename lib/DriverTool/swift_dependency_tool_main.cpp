//===--- language-dependency-tool.cpp - Convert binary languagedeps to YAML -----===//
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

#include "language/AST/FileSystem.h"
#include "language/AST/FineGrainedDependencies.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/FineGrainedDependencyFormat.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/SourceManager.h"
#include "language/Basic/ToolchainInitializer.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include "toolchain/Support/YAMLParser.h"
#include "toolchain/Support/YAMLTraits.h"

using namespace language;
using namespace fine_grained_dependencies;

//==============================================================================
// MARK: SourceFileDepGraph YAML reading & writing
//==============================================================================

// This introduces a redefinition wherever std::is_same_t<size_t, uint64_t>
// holds.
#if !(defined(__linux__) || defined(_WIN64) || defined(__FreeBSD__))
TOOLCHAIN_YAML_DECLARE_SCALAR_TRAITS(size_t, QuotingType::None)
#endif
TOOLCHAIN_YAML_DECLARE_ENUM_TRAITS(language::fine_grained_dependencies::NodeKind)
TOOLCHAIN_YAML_DECLARE_ENUM_TRAITS(language::fine_grained_dependencies::DeclAspect)
TOOLCHAIN_YAML_DECLARE_MAPPING_TRAITS(
    language::fine_grained_dependencies::DependencyKey)
TOOLCHAIN_YAML_DECLARE_MAPPING_TRAITS(language::fine_grained_dependencies::DepGraphNode)

namespace toolchain {
namespace yaml {
template <>
struct MappingContextTraits<
    language::fine_grained_dependencies::SourceFileDepGraphNode,
    language::fine_grained_dependencies::SourceFileDepGraph> {
  using SourceFileDepGraphNode =
      language::fine_grained_dependencies::SourceFileDepGraphNode;
  using SourceFileDepGraph =
      language::fine_grained_dependencies::SourceFileDepGraph;

  static void mapping(IO &io, SourceFileDepGraphNode &node,
                      SourceFileDepGraph &g);
};

template <>
struct SequenceTraits<
    std::vector<language::fine_grained_dependencies::SourceFileDepGraphNode *>> {
  using SourceFileDepGraphNode =
      language::fine_grained_dependencies::SourceFileDepGraphNode;
  using NodeVec = std::vector<SourceFileDepGraphNode *>;
  static size_t size(IO &, NodeVec &vec);
  static SourceFileDepGraphNode &element(IO &, NodeVec &vec, size_t index);
};

template <> struct ScalarTraits<language::Fingerprint> {
  static void output(const language::Fingerprint &fp, void *c, raw_ostream &os) {
    os << fp.getRawValue();
  }
  static StringRef input(StringRef s, void *, language::Fingerprint &fp) {
    if (auto convertedFP = language::Fingerprint::fromString(s))
      fp = convertedFP.value();
    else {
      toolchain::errs() << "Failed to convert fingerprint '" << s << "'\n";
      exit(1);
    }
    return StringRef();
  }
  static QuotingType mustQuote(StringRef S) { return needsQuotes(S); }
};

} // namespace yaml
} // namespace toolchain

TOOLCHAIN_YAML_DECLARE_MAPPING_TRAITS(
    language::fine_grained_dependencies::SourceFileDepGraph)

namespace toolchain {
namespace yaml {
// This introduces a redefinition wherever std::is_same_t<size_t, uint64_t>
// holds.
#if !(defined(__linux__) || defined(_WIN64) || defined(__FreeBSD__))
void ScalarTraits<size_t>::output(const size_t &Val, void *, raw_ostream &out) {
  out << Val;
}

StringRef ScalarTraits<size_t>::input(StringRef scalar, void *ctxt,
                                      size_t &value) {
  return scalar.getAsInteger(10, value) ? "could not parse size_t" : "";
}
#endif

void ScalarEnumerationTraits<language::fine_grained_dependencies::NodeKind>::
    enumeration(IO &io, language::fine_grained_dependencies::NodeKind &value) {
  using NodeKind = language::fine_grained_dependencies::NodeKind;
  io.enumCase(value, "topLevel", NodeKind::topLevel);
  io.enumCase(value, "nominal", NodeKind::nominal);
  io.enumCase(value, "potentialMember", NodeKind::potentialMember);
  io.enumCase(value, "member", NodeKind::member);
  io.enumCase(value, "dynamicLookup", NodeKind::dynamicLookup);
  io.enumCase(value, "externalDepend", NodeKind::externalDepend);
  io.enumCase(value, "sourceFileProvide", NodeKind::sourceFileProvide);
}

void ScalarEnumerationTraits<DeclAspect>::enumeration(
    IO &io, language::fine_grained_dependencies::DeclAspect &value) {
  using DeclAspect = language::fine_grained_dependencies::DeclAspect;
  io.enumCase(value, "interface", DeclAspect::interface);
  io.enumCase(value, "implementation", DeclAspect::implementation);
}

void MappingTraits<DependencyKey>::mapping(
    IO &io, language::fine_grained_dependencies::DependencyKey &key) {
  io.mapRequired("kind", key.kind);
  io.mapRequired("aspect", key.aspect);
  io.mapRequired("context", key.context);
  io.mapRequired("name", key.name);
}

void MappingTraits<DepGraphNode>::mapping(
    IO &io, language::fine_grained_dependencies::DepGraphNode &node) {
  io.mapRequired("key", node.key);
  io.mapOptional("fingerprint", node.fingerprint);
}

void MappingContextTraits<SourceFileDepGraphNode, SourceFileDepGraph>::mapping(
    IO &io, SourceFileDepGraphNode &node, SourceFileDepGraph &g) {
  MappingTraits<DepGraphNode>::mapping(io, node);
  io.mapRequired("sequenceNumber", node.sequenceNumber);
  std::vector<size_t> defsIDependUponVec(node.defsIDependUpon.begin(),
                                         node.defsIDependUpon.end());
  io.mapRequired("defsIDependUpon", defsIDependUponVec);
  io.mapRequired("isProvides", node.isProvides);
  if (!io.outputting()) {
    for (size_t u : defsIDependUponVec)
      node.defsIDependUpon.insert(u);
  }
  assert(g.getNode(node.sequenceNumber) && "Bad sequence number");
}

size_t SequenceTraits<std::vector<SourceFileDepGraphNode *>>::size(
    IO &, std::vector<SourceFileDepGraphNode *> &vec) {
  return vec.size();
}

SourceFileDepGraphNode &
SequenceTraits<std::vector<SourceFileDepGraphNode *>>::element(
    IO &, std::vector<SourceFileDepGraphNode *> &vec, size_t index) {
  while (vec.size() <= index)
    vec.push_back(new SourceFileDepGraphNode());
  return *vec[index];
}

void MappingTraits<SourceFileDepGraph>::mapping(IO &io, SourceFileDepGraph &g) {
  io.mapRequired("allNodes", g.allNodes, g);
}
} // namespace yaml
} // namespace toolchain

enum class ActionType : unsigned {
  None,
  BinaryToYAML,
  YAMLToBinary
};

int language_dependency_tool_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  toolchain::cl::OptionCategory Category("language-dependency-tool Options");

  toolchain::cl::opt<std::string>
  InputFilename("input-filename",
                toolchain::cl::desc("Name of the input file"),
                toolchain::cl::cat(Category));

  toolchain::cl::opt<std::string>
  OutputFilename("output-filename",
                 toolchain::cl::desc("Name of the output file"),
                 toolchain::cl::cat(Category));

  toolchain::cl::opt<ActionType>
  Action(toolchain::cl::desc("Mode:"), toolchain::cl::init(ActionType::None),
         toolchain::cl::cat(Category),
         toolchain::cl::values(
             clEnumValN(ActionType::BinaryToYAML,
                        "to-yaml", "Convert new binary .codedeps format to YAML"),
             clEnumValN(ActionType::YAMLToBinary,
                        "from-yaml", "Convert YAML to new binary .codedeps format")));

  toolchain::cl::HideUnrelatedOptions(Category);
  toolchain::cl::ParseCommandLineOptions(argv.size(), argv.data(), "Codira Dependency Tool\n");

  SourceManager sourceMgr;
  DiagnosticEngine diags(sourceMgr);
  toolchain::vfs::OnDiskOutputBackend outputBackend;

  switch (Action) {
  case ActionType::None: {
    toolchain::errs() << "action required\n";
    toolchain::cl::PrintHelpMessage();
    return 1;
  }

  case ActionType::BinaryToYAML: {
    auto fg = SourceFileDepGraph::loadFromPath(InputFilename, true);
    if (!fg) {
      toolchain::errs() << "Failed to read dependency file\n";
      return 1;
    }

    bool hadError =
      withOutputPath(diags, outputBackend, OutputFilename,
        [&](toolchain::raw_pwrite_stream &out) {
          out << "# Fine-grained v0\n";
          toolchain::yaml::Output yamlWriter(out);
          yamlWriter << *fg;
          return false;
        });

    if (hadError) {
      toolchain::errs() << "Failed to write YAML languagedeps\n";
    }
    break;
  }

  case ActionType::YAMLToBinary: {
    auto bufferOrError = toolchain::MemoryBuffer::getFile(InputFilename);
    if (!bufferOrError) {
      toolchain::errs() << "Failed to read dependency file\n";
      return 1;
    }

    auto &buffer = *bufferOrError.get();

    SourceFileDepGraph fg;
    toolchain::yaml::Input yamlReader(toolchain::MemoryBufferRef(buffer), nullptr);
    yamlReader >> fg;
    if (yamlReader.error()) {
      toolchain::errs() << "Failed to parse YAML languagedeps\n";
      return 1;
    }

    if (writeFineGrainedDependencyGraphToPath(
            diags, outputBackend, OutputFilename, fg)) {
      toolchain::errs() << "Failed to write binary languagedeps\n";
      return 1;
    }

    break;
  }
  }

  return 0;
}
