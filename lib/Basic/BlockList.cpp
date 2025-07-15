//===--- BlockList.cpp - BlockList utilities ------------------------------===//
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

#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/YAMLParser.h"
#include "toolchain/Support/YAMLTraits.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/BlockList.h"
#include "language/Basic/SourceManager.h"

struct language::BlockListStore::Implementation {
  SourceManager SM;
  toolchain::StringMap<std::vector<BlockListAction>> ModuleActionDict;
  toolchain::StringMap<std::vector<BlockListAction>> ProjectActionDict;
  void addConfigureFilePath(StringRef path);
  bool hasBlockListAction(StringRef key, BlockListKeyKind keyKind,
                          BlockListAction action);
  void collectBlockList(toolchain::yaml::Node *N, BlockListAction action);

  toolchain::StringMap<std::vector<BlockListAction>> *getDictToUse(BlockListKeyKind kind) {
    switch (kind) {
    case BlockListKeyKind::ModuleName:
      return &ModuleActionDict;
    case BlockListKeyKind::ProjectName:
      return &ProjectActionDict;
    case BlockListKeyKind::Undefined:
      return nullptr;
    }
  }
  static std::string getScalaString(toolchain::yaml::Node *N) {
    toolchain::SmallString<64> Buffer;
    if (auto *scala = dyn_cast<toolchain::yaml::ScalarNode>(N)) {
      return scala->getValue(Buffer).str();
    }
    return std::string();
  }

  Implementation(SourceManager &SM) : SM(SM.getFileSystem()) {}
};

language::BlockListStore::BlockListStore(language::SourceManager &SM)
    : Impl(*new Implementation(SM)) {}

language::BlockListStore::~BlockListStore() { delete &Impl; }

bool language::BlockListStore::hasBlockListAction(StringRef key,
    BlockListKeyKind keyKind, BlockListAction action) {
  return Impl.hasBlockListAction(key, keyKind, action);
}

void language::BlockListStore::addConfigureFilePath(StringRef path) {
  Impl.addConfigureFilePath(path);
}

bool language::BlockListStore::Implementation::hasBlockListAction(StringRef key,
    BlockListKeyKind keyKind, BlockListAction action) {
  auto *dict = getDictToUse(keyKind);
  assert(dict);
  auto it = dict->find(key);
  if (it == dict->end())
    return false;
  return toolchain::is_contained(it->second, action);
}

void language::BlockListStore::Implementation::collectBlockList(toolchain::yaml::Node *N,
                                                      BlockListAction action) {
  namespace yaml = toolchain::yaml;
  auto *pair = dyn_cast<yaml::KeyValueNode>(N);
  if (!pair)
    return;
  std::string rawKey = getScalaString(pair->getKey());
  auto keyKind = toolchain::StringSwitch<BlockListKeyKind>(rawKey)
#define CASE(X) .Case(#X, BlockListKeyKind::X)
    CASE(ModuleName)
    CASE(ProjectName)
#undef CASE
    .Default(BlockListKeyKind::Undefined);
  if (keyKind == BlockListKeyKind::Undefined)
    return;
  auto *dictToUse = getDictToUse(keyKind);
  assert(dictToUse);
  auto *seq = dyn_cast<yaml::SequenceNode>(pair->getValue());
  if (!seq)
    return;
  for (auto &node: *seq) {
    std::string name = getScalaString(&node);
    dictToUse->insert({name, std::vector<BlockListAction>()})
      .first->second.push_back(action);
  }
}

void language::BlockListStore::Implementation::addConfigureFilePath(StringRef path) {
  namespace yaml = toolchain::yaml;

  // Load the input file.
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> FileBufOrErr =
    vfs::getFileOrSTDIN(*SM.getFileSystem(), path,
                        /*FileSize*/-1, /*RequiresNullTerminator*/true,
                        /*IsVolatile*/false, /*RetryCount*/30);
  if (!FileBufOrErr) {
    return;
  }
  StringRef Buffer = FileBufOrErr->get()->getBuffer();
  yaml::Stream Stream(toolchain::MemoryBufferRef(Buffer, path),
                      SM.getLLVMSourceMgr());
  for (auto DI = Stream.begin(); DI != Stream.end(); ++ DI) {
    assert(DI != Stream.end() && "Failed to read a document");
    auto *MapNode = dyn_cast<yaml::MappingNode>(DI->getRoot());
    if (!MapNode)
      continue;
    for (auto &pair: *MapNode) {
      std::string key = getScalaString(pair.getKey());
      auto action = toolchain::StringSwitch<BlockListAction>(key)
#define BLOCKLIST_ACTION(X) .Case(#X, BlockListAction::X)
#include "language/Basic/BlockListAction.def"
        .Default(BlockListAction::Undefined);
      if (action == BlockListAction::Undefined)
        continue;
      auto *map = dyn_cast<yaml::MappingNode>(pair.getValue());
      if (!map)
        continue;
      for (auto &innerPair: *map) {
        collectBlockList(&innerPair, action);
      }
    }
  }
}
