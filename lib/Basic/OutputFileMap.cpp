//===--- OutputFileMap.cpp - Map of inputs to multiple outputs ------------===//
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

#include "language/Basic/OutputFileMap.h"
#include "language/Basic/FileTypes.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include <system_error>

using namespace language;

toolchain::Expected<OutputFileMap>
OutputFileMap::loadFromPath(StringRef Path, StringRef workingDirectory) {
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
      toolchain::MemoryBuffer::getFile(Path);
  if (!FileBufOrErr) {
    return toolchain::errorCodeToError(FileBufOrErr.getError());
  }
  return loadFromBuffer(std::move(FileBufOrErr.get()), workingDirectory);
}

toolchain::Expected<OutputFileMap>
OutputFileMap::loadFromBuffer(StringRef Data, StringRef workingDirectory) {
  std::unique_ptr<toolchain::MemoryBuffer> Buffer{
      toolchain::MemoryBuffer::getMemBuffer(Data)};
  return loadFromBuffer(std::move(Buffer), workingDirectory);
}

toolchain::Expected<OutputFileMap>
OutputFileMap::loadFromBuffer(std::unique_ptr<toolchain::MemoryBuffer> Buffer,
                              StringRef workingDirectory) {
  return parse(std::move(Buffer), workingDirectory);
}

const TypeToPathMap *OutputFileMap::getOutputMapForInput(StringRef Input) const{
  auto iter = InputToOutputsMap.find(Input);
  if (iter == InputToOutputsMap.end())
    return nullptr;
  else
    return &iter->second;
}

TypeToPathMap &
OutputFileMap::getOrCreateOutputMapForInput(StringRef Input) {
  return InputToOutputsMap[Input];
}

const TypeToPathMap *OutputFileMap::getOutputMapForSingleOutput() const {
  return getOutputMapForInput(StringRef());
}

TypeToPathMap &
OutputFileMap::getOrCreateOutputMapForSingleOutput() {
  return InputToOutputsMap[StringRef()];
}

void OutputFileMap::dump(toolchain::raw_ostream &os, bool Sort) const {
  using TypePathPair = std::pair<file_types::ID, std::string>;

  auto printOutputPair = [&os](StringRef InputPath,
                               const TypePathPair &OutputPair) -> void {
    os << InputPath << " -> " << file_types::getTypeName(OutputPair.first)
       << ": \"" << OutputPair.second << "\"\n";
  };

  if (Sort) {
    using PathMapPair = std::pair<StringRef, TypeToPathMap>;
    std::vector<PathMapPair> Maps;
    for (auto &InputPair : InputToOutputsMap) {
      Maps.emplace_back(InputPair.first(), InputPair.second);
    }
    std::sort(Maps.begin(), Maps.end(), [] (const PathMapPair &LHS,
                                            const PathMapPair &RHS) -> bool {
      return LHS.first < RHS.first;
    });
    for (auto &InputPair : Maps) {
      const TypeToPathMap &Map = InputPair.second;
      std::vector<TypePathPair> Pairs;
      Pairs.insert(Pairs.end(), Map.begin(), Map.end());
      std::sort(Pairs.begin(), Pairs.end(), [](const TypePathPair &LHS,
                                               const TypePathPair &RHS) -> bool {
        return LHS < RHS;
      });
      for (auto &OutputPair : Pairs) {
        printOutputPair(InputPair.first, OutputPair);
      }
    }
  } else {
    for (auto &InputPair : InputToOutputsMap) {
      const TypeToPathMap &Map = InputPair.second;
      for (const TypePathPair &OutputPair : Map) {
        printOutputPair(InputPair.first(), OutputPair);
      }
    }
  }
}

static void writeQuotedEscaped(toolchain::raw_ostream &os,
                               const StringRef fileName) {
  os << "\"" << toolchain::yaml::escape(fileName) << "\"";
}

void OutputFileMap::write(toolchain::raw_ostream &os,
                          ArrayRef<StringRef> inputs) const {
  for (const auto &input : inputs) {
    writeQuotedEscaped(os, input);
    os << ":";

    const TypeToPathMap *outputMap = getOutputMapForInput(input);
    if (!outputMap) {
      // The map doesn't have an entry for this input. (Perhaps there were no
      // outputs and thus the entry was never created.) Put an empty sub-map
      // into the output and move on.
      os << " {}\n";
      continue;
    }

    os << "\n";
    // DenseMap is unordered. If you write a test, please sort the output.
    for (auto &typeAndOutputPath : *outputMap) {
      file_types::ID type = typeAndOutputPath.getFirst();
      StringRef output = typeAndOutputPath.getSecond();
      os << "  " << file_types::getTypeName(type) << ": ";
      writeQuotedEscaped(os, output);
      os << "\n";
    }
  }
}

toolchain::Expected<OutputFileMap>
OutputFileMap::parse(std::unique_ptr<toolchain::MemoryBuffer> Buffer,
                     StringRef workingDirectory) {
  auto constructError =
      [](const char *errorString) -> toolchain::Expected<OutputFileMap> {
    return toolchain::make_error<toolchain::StringError>(errorString,
                                               toolchain::inconvertibleErrorCode());
  };
  /// FIXME: Make the returned error strings more specific by including some of
  /// the source.
  toolchain::SourceMgr SM;
  toolchain::yaml::Stream YAMLStream(Buffer->getMemBufferRef(), SM);
  auto I = YAMLStream.begin();
  if (I == YAMLStream.end())
    return constructError("empty YAML stream");

  auto Root = I->getRoot();
  if (!Root)
    return constructError("no root");

  OutputFileMap OFM;

  auto *Map = dyn_cast<toolchain::yaml::MappingNode>(Root);
  if (!Map)
    return constructError("root was not a MappingNode");

  auto resolvePath =
      [workingDirectory](
          toolchain::yaml::ScalarNode *Path,
          toolchain::SmallVectorImpl<char> &PathStorage) -> StringRef {
    StringRef PathStr = Path->getValue(PathStorage);
    if (workingDirectory.empty() || PathStr.empty() || PathStr == "-" ||
        toolchain::sys::path::is_absolute(PathStr)) {
      return PathStr;
    }
    // Copy the path to avoid making assumptions about how getValue deals with
    // Storage.
    SmallString<128> PathStrCopy(PathStr);
    PathStorage.clear();
    PathStorage.reserve(PathStrCopy.size() + workingDirectory.size() + 1);
    PathStorage.insert(PathStorage.begin(), workingDirectory.begin(),
                       workingDirectory.end());
    toolchain::sys::path::append(PathStorage, PathStrCopy);
    return StringRef(PathStorage.data(), PathStorage.size());
  };

  for (auto &Pair : *Map) {
    toolchain::yaml::Node *Key = Pair.getKey();
    toolchain::yaml::Node *Value = Pair.getValue();

    if (!Key)
      return constructError("bad key");

    if (!Value)
      return constructError("bad value");

    auto *InputPath = dyn_cast<toolchain::yaml::ScalarNode>(Key);
    if (!InputPath)
      return constructError("input path not a scalar node");

    toolchain::yaml::MappingNode *OutputMapNode =
      dyn_cast<toolchain::yaml::MappingNode>(Value);
    if (!OutputMapNode)
      return constructError("output map not a MappingNode");

    TypeToPathMap OutputMap;

    for (auto &OutputPair : *OutputMapNode) {
      toolchain::yaml::Node *Key = OutputPair.getKey();
      toolchain::yaml::Node *Value = OutputPair.getValue();

      if (!Key)
        return constructError("bad kind");

      if (!Value)
        return constructError("bad path");

      auto *KindNode = dyn_cast<toolchain::yaml::ScalarNode>(Key);
      if (!KindNode)
        return constructError("kind not a ScalarNode");

      auto *Path = dyn_cast<toolchain::yaml::ScalarNode>(Value);
      if (!Path)
        return constructError("path not a scalar node");

      toolchain::SmallString<16> KindStorage;
      file_types::ID Kind =
          file_types::lookupTypeForName(KindNode->getValue(KindStorage));

      // Ignore unknown types, so that an older languagec can be used with a newer
      // build system.
      if (Kind == file_types::TY_INVALID)
        continue;

      toolchain::SmallString<128> PathStorage;
      OutputMap.insert(std::pair<file_types::ID, std::string>(
          Kind, resolvePath(Path, PathStorage).str()));
    }

    toolchain::SmallString<128> InputStorage;
    OFM.InputToOutputsMap[resolvePath(InputPath, InputStorage)] =
        std::move(OutputMap);
  }

  if (YAMLStream.failed())
    return constructError("Output file map parse failed");

  return OFM;
}
