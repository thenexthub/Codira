//===--- language-demangle-yamldump.cpp --------------------------------------===//
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
/// \file
///
/// This tool is similar to the demangler but is intended to /not/ be installed
/// into the OS. This means that it can link against toolchain support and friends
/// and thus provide extra functionality. Today it is only used to dump the
/// demangler tree in YAML form for easy processing.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/Toolchain.h"
#include "language/Demangling/Demangle.h"
#include "language/Demangling/ManglingMacros.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include "toolchain/Support/Regex.h"
#include "toolchain/Support/Signals.h"
#include "toolchain/Support/raw_ostream.h"

// For std::rand, to work around a bug if main()'s first function call passes
// argv[0].
#if defined(__CYGWIN__)
#include <cstdlib>
#endif

#include <iostream>

using namespace language;
using namespace language::Demangle;

//===----------------------------------------------------------------------===//
//                          YAML Dump Implementation
//===----------------------------------------------------------------------===//

namespace {

using toolchain::yaml::IO;
using toolchain::yaml::MappingTraits;
using toolchain::yaml::Output;
using toolchain::yaml::ScalarEnumerationTraits;
using toolchain::yaml::SequenceTraits;

struct YAMLNode {
  Node::Kind kind;
  std::vector<YAMLNode *> children;
  StringRef name = StringRef();

  YAMLNode(Node::Kind kind) : kind(kind), children() {}
};

} // end anonymous namespace

namespace toolchain {
namespace yaml {

template <> struct ScalarEnumerationTraits<language::Demangle::Node::Kind> {
  static void enumeration(IO &io, language::Demangle::Node::Kind &value) {
#define NODE(ID) io.enumCase(value, #ID, language::Demangle::Node::Kind::ID);
#include "language/Demangling/DemangleNodes.def"
  }
};

template <> struct MappingTraits<YAMLNode *> {
  static void mapping(IO &io, YAMLNode *&node) {
    io.mapOptional("name", node->name);
    io.mapRequired("kind", node->kind);
    io.mapRequired("children", node->children);
  }
};

template <> struct MappingTraits<YAMLNode> {
  static void mapping(IO &io, YAMLNode &node) {
    io.mapOptional("name", node.name);
    io.mapRequired("kind", node.kind);
    io.mapRequired("children", node.children);
  }
};

} // namespace yaml
} // namespace toolchain

TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(YAMLNode *)

static std::string getNodeTreeAsYAML(toolchain::StringRef name,
				     NodePointer root) {
  std::vector<std::unique_ptr<YAMLNode>> nodes;

  std::vector<std::pair<NodePointer, YAMLNode *>> worklist;
  nodes.emplace_back(new YAMLNode(root->getKind()));
  nodes.back()->name = name;
  worklist.emplace_back(root, &*nodes.back());

  while (!worklist.empty()) {
    NodePointer np;
    YAMLNode *node;
    std::tie(np, node) = worklist.back();
    worklist.pop_back();

    for (unsigned i = 0; i < np->getNumChildren(); ++i) {
      nodes.emplace_back(new YAMLNode(np->getChild(i)->getKind()));
      node->children.emplace_back(&*nodes.back());
    }
  }

  std::string output;
  toolchain::raw_string_ostream stream(output);
  toolchain::yaml::Output yout(stream);
  yout << *nodes.front();
  return stream.str();
}

//===----------------------------------------------------------------------===//
//                            Top Level Entrypoint
//===----------------------------------------------------------------------===//

static toolchain::cl::opt<bool>
    DisableSugar("no-sugar",
                 toolchain::cl::desc("No sugar mode (disable common language idioms "
                                "such as ? and [] from the output)"));

static toolchain::cl::opt<bool> Simplified(
    "simplified",
    toolchain::cl::desc("Don't display module names or implicit self types"));

static toolchain::cl::list<std::string>
    InputNames(toolchain::cl::Positional, toolchain::cl::desc("[mangled name...]"),
               toolchain::cl::ZeroOrMore);

static toolchain::StringRef substrBefore(toolchain::StringRef whole,
                                    toolchain::StringRef part) {
  return whole.slice(0, part.data() - whole.data());
}

static toolchain::StringRef substrAfter(toolchain::StringRef whole,
                                   toolchain::StringRef part) {
  return whole.substr((part.data() - whole.data()) + part.size());
}

static void demangle(toolchain::raw_ostream &os, toolchain::StringRef name,
                     language::Demangle::Context &DCtx,
                     const language::Demangle::DemangleOptions &options) {
  if (name.starts_with("__")) {
    name = name.substr(1);
  }
  language::Demangle::NodePointer pointer = DCtx.demangleSymbolAsNode(name);
  // We do not emit a message so that we end up dumping
  toolchain::outs() << getNodeTreeAsYAML(name, pointer);
  DCtx.clear();
}

static int demangleSTDIN(const language::Demangle::DemangleOptions &options) {
  // This doesn't handle Unicode symbols, but maybe that's okay.
  // Also accept the future mangling prefix.
  toolchain::Regex maybeSymbol("(_T|_?\\$[Ss])[_a-zA-Z0-9$.]+");

  language::Demangle::Context DCtx;
  for (std::string mangled; std::getline(std::cin, mangled);) {
    toolchain::StringRef inputContents(mangled);

    toolchain::SmallVector<toolchain::StringRef, 1> matches;
    while (maybeSymbol.match(inputContents, &matches)) {
      toolchain::outs() << substrBefore(inputContents, matches.front());
      demangle(toolchain::outs(), matches.front(), DCtx, options);
      inputContents = substrAfter(inputContents, matches.front());
    }

    toolchain::errs() << "Failed to match: " << inputContents << '\n';
  }

  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
#if defined(__CYGWIN__)
  // Cygwin clang 3.5.2 with '-O3' generates CRASHING BINARY,
  // if main()'s first function call is passing argv[0].
  std::rand();
#endif
  toolchain::cl::ParseCommandLineOptions(argc, argv);

  language::Demangle::DemangleOptions options;
  options.SynthesizeSugarOnTypes = !DisableSugar;
  if (Simplified)
    options = language::Demangle::DemangleOptions::SimplifiedUIDemangleOptions();

  if (InputNames.empty()) {
    return demangleSTDIN(options);
  } else {
    language::Demangle::Context DCtx;
    for (toolchain::StringRef name : InputNames) {
      if (name.starts_with("S")) {
        std::string correctedName = std::string("$") + name.str();
        demangle(toolchain::outs(), correctedName, DCtx, options);
      } else {
        demangle(toolchain::outs(), name, DCtx, options);
      }
      toolchain::outs() << '\n';
    }

    return EXIT_SUCCESS;
  }
}
