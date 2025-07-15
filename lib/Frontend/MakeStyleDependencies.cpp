//===--- MakeStyleDependencies.cpp -- Emit make-style dependencies --------===//
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

#include "language/Frontend/MakeStyleDependencies.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/ModuleLoader.h"
#include "language/Basic/SourceLoc.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/FrontendOptions.h"
#include "language/Frontend/InputFile.h"
#include "language/FrontendTool/FrontendTool.h"

#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/PrefixMapper.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

StringRef
language::frontend::utils::escapeForMake(StringRef raw,
                                      toolchain::SmallVectorImpl<char> &buffer) {
  buffer.clear();

  // The escaping rules for GNU make are complicated due to the various
  // substitutions and use of the tab in the leading position for recipes.
  // Various symbols have significance in different contexts.  It is not
  // possible to correctly quote all characters in Make (as of 3.7).  Match
  // gcc and clang's behaviour for the escaping which covers only a subset of
  // characters.
  for (unsigned I = 0, E = raw.size(); I != E; ++I) {
    switch (raw[I]) {
    case '#': // Handle '#' the broken GCC way
      buffer.push_back('\\');
      break;

    case ' ':
      for (unsigned J = I; J && raw[J - 1] == '\\'; --J)
        buffer.push_back('\\');
      buffer.push_back('\\');
      break;

    case '$': // $ is escaped by $
      buffer.push_back('$');
      break;
    }
    buffer.push_back(raw[I]);
  }
  buffer.push_back('\0');

  return buffer.data();
}

/// This sorting function is used to stabilize the order in which dependencies
/// are emitted into \c .d files that are consumed by external build systems.
/// This serves to eliminate order as a source of non-determinism in these
/// outputs.
///
/// The exact sorting predicate is not important. Currently, it is a
/// lexicographic comparison that reverses the provided strings before applying
/// the sorting predicate. This has the benefit of being somewhat
/// invariant with respect to the installation location of various system
/// components. e.g. on two systems, the same file identified by two different
/// paths differing only in their relative install location such as
///
/// /Applications/MyXcode.app/Path/To/A/Framework/In/The/SDK/Header.h
/// /Applications/Xcodes/AnotherXcode.app/Path/To/A/Framework/In/The/SDK/Header.h
///
/// should appear in roughly the same order relative to other paths. Ultimately,
/// this makes it easier to test the contents of the emitted files with tools
/// like FileCheck.
template <typename Container>
static std::vector<std::string>
reversePathSortedFilenames(const Container &elts) {
  std::vector<std::string> tmp(elts.begin(), elts.end());
  std::sort(tmp.begin(), tmp.end(),
            [](const std::string &a, const std::string &b) -> bool {
              return std::lexicographical_compare(a.rbegin(), a.rend(),
                                                  b.rbegin(), b.rend());
            });
  return tmp;
}

static void emitMakeDependenciesFile(std::vector<std::string> &dependencies,
                                     const FrontendOptions &opts,
                                     const InputFile &input,
                                     const std::vector<std::string> &prefixMap,
                                     toolchain::raw_ostream &os) {
  // Prefix map all the path if needed.
  if (!prefixMap.empty()) {
    SmallVector<toolchain::MappedPrefix, 4> prefixes;
    toolchain::MappedPrefix::transformJoinedIfValid(prefixMap, prefixes);
    toolchain::PrefixMapper mapper;
    mapper.addRange(prefixes);
    mapper.sort();
    toolchain::for_each(dependencies, [&mapper](std::string &dep) {
      dep = mapper.mapToString(dep);
    });
  }

  // Escape all the dependencies.
  toolchain::SmallString<256> buffer;
  toolchain::for_each(dependencies, [&buffer](std::string &dep) {
    dep = frontend::utils::escapeForMake(dep, buffer).str();
  });

  // FIXME: Xcode can't currently handle multiple targets in a single
  // dependency line.
  opts.forAllOutputPaths(input, [&](const StringRef path) {
    os << frontend::utils::escapeForMake(path, buffer) << " :";
    for (auto &dep : dependencies)
      os << ' ' << dep;
    os << "\n";
  });
}

static toolchain::SmallString<256>
serializeDependencies(const std::vector<std::string> &deps) {
  // Encode the dependencies as null terminated strings.
  toolchain::SmallString<256> buffer;
  for (auto &dep : deps) {
    buffer.append(dep);
    buffer.push_back('\0');
  }

  return buffer;
}

static std::vector<std::string> deserializeDependencies(StringRef buffer) {
  std::vector<std::string> deps;
  StringRef dep;
  while (!buffer.empty()) {
    std::tie(dep, buffer) = buffer.split('\0');
    deps.push_back(dep.str());
  }

  return deps;
}

bool language::emitMakeDependenciesFromSerializedBuffer(
    toolchain::StringRef buffer, toolchain::raw_ostream &os, const FrontendOptions &opts,
    const InputFile &input, DiagnosticEngine &diags) {
  auto dependencies = deserializeDependencies(buffer);

  emitMakeDependenciesFile(dependencies, opts, input, opts.CacheReplayPrefixMap,
                           os);
  return false;
}

/// Emits a Make-style dependencies file.
bool language::emitMakeDependenciesIfNeeded(CompilerInstance &instance,
                                         const InputFile &input) {
  auto &opts = instance.getInvocation().getFrontendOptions();
  auto *depTracker = instance.getDependencyTracker();
  auto dependenciesFilePath = input.getDependenciesFilePath();
  if (dependenciesFilePath.empty())
    return false;

  // collect everything in memory to avoid redundant work
  // when there are multiple targets
  std::vector<std::string> dependencies;

  // First include all other files in the module. Make-style dependencies
  // need to be conservative!
  auto inputPaths =
      reversePathSortedFilenames(opts.InputsAndOutputs.getInputFilenames());
  for (auto const &path : inputPaths) {
    dependencies.push_back(path);
  }
  // Then print dependencies we've picked up during compilation.
  auto dependencyPaths =
      reversePathSortedFilenames(depTracker->getDependencies());
  for (auto const &path : dependencyPaths) {
    dependencies.push_back(path);
  }
  auto incrementalDependencyPaths =
      reversePathSortedFilenames(depTracker->getIncrementalDependencyPaths());
  for (auto const &path : incrementalDependencyPaths) {
    dependencies.push_back(path);
  }
  auto macroPluginDependencyPath =
      reversePathSortedFilenames(depTracker->getMacroPluginDependencyPaths());
  for (auto const &path : macroPluginDependencyPath) {
    dependencies.push_back(path);
  }

  auto out = instance.getOutputBackend().createFile(dependenciesFilePath);
  if (!out) {
    instance.getDiags().diagnose(SourceLoc(), diag::error_opening_output,
                                 dependenciesFilePath,
                                 toString(out.takeError()));
    return true;
  }

  emitMakeDependenciesFile(dependencies, opts, input, opts.CacheReplayPrefixMap,
                           *out);

  if (auto error = out->keep()) {
    instance.getDiags().diagnose(SourceLoc(), diag::error_closing_output,
                                 dependenciesFilePath,
                                 toString(std::move(error)));
    return true;
  }

  // If store into CAS if needed. This has to happen after emitted the real file
  // so the CASBackend is not finalized before emitting the real file.
  if (instance.supportCaching()) {
    auto &CASBackend = instance.getCASOutputBackend();
    auto buffer = serializeDependencies(dependencies);
    if (auto err = CASBackend.storeMakeDependenciesFile(dependenciesFilePath,
                                                        buffer)) {
      instance.getDiags().diagnose(SourceLoc(), diag::error_cas,
                                   "storing make-style dependency file",
                                   toString(std::move(err)));
      return true;
    }
  }

  return false;
}
