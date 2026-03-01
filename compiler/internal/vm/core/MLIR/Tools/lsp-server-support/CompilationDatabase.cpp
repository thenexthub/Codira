//===- CompilationDatabase.cpp - LSP Compilation Database -----------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "mlir/Tools/lsp-server-support/CompilationDatabase.h"
#include "mlir/Support/FileUtilities.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/LSP/Logging.h"
#include "vm/core/Support/LSP/Protocol.h"
#include "vm/core/Support/YAMLTraits.h"

using namespace mlir;
using namespace mlir::lsp;
using toolchain::lsp::Logger;

//===----------------------------------------------------------------------===//
// YamlFileInfo
//===----------------------------------------------------------------------===//

namespace {
struct YamlFileInfo {
  /// The absolute path to the file.
  std::string filename;
  /// The include directories available for the file.
  std::vector<std::string> includeDirs;
};
} // namespace

//===----------------------------------------------------------------------===//
// CompilationDatabase
//===----------------------------------------------------------------------===//

LLVM_YAML_IS_DOCUMENT_LIST_VECTOR(YamlFileInfo)

namespace vm::core {
namespace yaml {
template <>
struct MappingTraits<YamlFileInfo> {
  static void mapping(IO &io, YamlFileInfo &info) {
    // Parse the filename and normalize it to the form we will expect from
    // incoming URIs.
    io.mapRequired("filepath", info.filename);

    // Normalize the filename to avoid incompatability with incoming URIs.
    if (Expected<lsp::URIForFile> uri =
            lsp::URIForFile::fromFile(info.filename))
      info.filename = uri->file().str();

    // Parse the includes from the yaml stream. These are in the form of a
    // semi-colon delimited list.
    std::string combinedIncludes;
    io.mapRequired("includes", combinedIncludes);
    for (StringRef include : toolchain::split(combinedIncludes, ";")) {
      if (!include.empty())
        info.includeDirs.push_back(include.str());
    }
  }
};
} // end namespace yaml
} // end namespace vm::core

CompilationDatabase::CompilationDatabase(ArrayRef<std::string> databases) {
  for (StringRef filename : databases)
    loadDatabase(filename);
}

const CompilationDatabase::FileInfo &
CompilationDatabase::getFileInfo(StringRef filename) const {
  auto it = files.find(filename);
  return it == files.end() ? defaultFileInfo : it->second;
}

void CompilationDatabase::loadDatabase(StringRef filename) {
  if (filename.empty())
    return;

  // Set up the input file.
  std::string errorMessage;
  std::unique_ptr<toolchain::MemoryBuffer> inputFile =
      openInputFile(filename, &errorMessage);
  if (!inputFile) {
    Logger::error("Failed to open compilation database: {0}", errorMessage);
    return;
  }
  toolchain::yaml::Input yaml(inputFile->getBuffer());

  // Parse the yaml description and add any new files to the database.
  std::vector<YamlFileInfo> parsedFiles;
  yaml >> parsedFiles;

  SetVector<StringRef> knownIncludes;
  for (auto &file : parsedFiles) {
    auto it = files.try_emplace(file.filename, std::move(file.includeDirs));

    // If we encounter a duplicate file, log a warning and ignore it.
    if (!it.second) {
      Logger::info("Duplicate file in compilation database: {0}",
                   file.filename);
      continue;
    }

    // Track the includes for the file.
    knownIncludes.insert_range(it.first->second.includeDirs);
  }

  // Add all of the known includes to the default file info. We don't know any
  // information about how to treat these files, but these may be project files
  // that we just don't yet have information for. In these cases, providing some
  // heuristic information provides a better user experience, and generally
  // shouldn't lead to any negative side effects.
  for (StringRef include : knownIncludes)
    defaultFileInfo.includeDirs.push_back(include.str());
}
