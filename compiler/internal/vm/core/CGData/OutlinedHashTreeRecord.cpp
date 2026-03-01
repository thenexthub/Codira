//===-- OutlinedHashTreeRecord.cpp ----------------------------------------===//
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
//
// This defines the OutlinedHashTreeRecord class. This class holds the outlined
// hash tree for both serialization and deserialization processes. It utilizes
// two data formats for serialization: raw binary data and YAML.
// These two formats can be used interchangeably.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CGData/OutlinedHashTreeRecord.h"
#include "vm/core/ObjectYAML/YAML.h"
#include "vm/core/Support/Endian.h"
#include "vm/core/Support/EndianStream.h"

#define DEBUG_TYPE "outlined-hash-tree"

using namespace vm::core;
using namespace vm::core::support;

namespace vm::core {
namespace yaml {

template <> struct MappingTraits<HashNodeStable> {
  static void mapping(IO &io, HashNodeStable &res) {
    io.mapRequired("Hash", res.Hash);
    io.mapRequired("Terminals", res.Terminals);
    io.mapRequired("SuccessorIds", res.SuccessorIds);
  }
};

template <> struct CustomMappingTraits<IdHashNodeStableMapTy> {
  static void inputOne(IO &io, StringRef Key, IdHashNodeStableMapTy &V) {
    HashNodeStable NodeStable;
    io.mapRequired(Key, NodeStable);
    unsigned Id;
    if (Key.getAsInteger(0, Id)) {
      io.setError("Id not an integer");
      return;
    }
    V.insert({Id, NodeStable});
  }

  static void output(IO &io, IdHashNodeStableMapTy &V) {
    for (auto Iter = V.begin(); Iter != V.end(); ++Iter)
      io.mapRequired(utostr(Iter->first), Iter->second);
  }
};

} // namespace yaml
} // namespace vm::core

void OutlinedHashTreeRecord::serialize(raw_ostream &OS) const {
  IdHashNodeStableMapTy IdNodeStableMap;
  convertToStableData(IdNodeStableMap);
  support::endian::Writer Writer(OS, endianness::little);
  Writer.write<uint32_t>(IdNodeStableMap.size());

  for (const auto &[Id, NodeStable] : IdNodeStableMap) {
    Writer.write<uint32_t>(Id);
    Writer.write<uint64_t>(NodeStable.Hash);
    Writer.write<uint32_t>(NodeStable.Terminals);
    Writer.write<uint32_t>(NodeStable.SuccessorIds.size());
    for (auto SuccessorId : NodeStable.SuccessorIds)
      Writer.write<uint32_t>(SuccessorId);
  }
}

void OutlinedHashTreeRecord::deserialize(const unsigned char *&Ptr) {
  IdHashNodeStableMapTy IdNodeStableMap;
  auto NumIdNodeStableMap =
      endian::readNext<uint32_t, endianness::little, unaligned>(Ptr);

  for (unsigned I = 0; I < NumIdNodeStableMap; ++I) {
    auto Id = endian::readNext<uint32_t, endianness::little, unaligned>(Ptr);
    HashNodeStable NodeStable;
    NodeStable.Hash =
        endian::readNext<uint64_t, endianness::little, unaligned>(Ptr);
    NodeStable.Terminals =
        endian::readNext<uint32_t, endianness::little, unaligned>(Ptr);
    auto NumSuccessorIds =
        endian::readNext<uint32_t, endianness::little, unaligned>(Ptr);
    for (unsigned J = 0; J < NumSuccessorIds; ++J)
      NodeStable.SuccessorIds.push_back(
          endian::readNext<uint32_t, endianness::little, unaligned>(Ptr));

    IdNodeStableMap[Id] = std::move(NodeStable);
  }

  convertFromStableData(IdNodeStableMap);
}

void OutlinedHashTreeRecord::serializeYAML(yaml::Output &YOS) const {
  IdHashNodeStableMapTy IdNodeStableMap;
  convertToStableData(IdNodeStableMap);

  YOS << IdNodeStableMap;
}

void OutlinedHashTreeRecord::deserializeYAML(yaml::Input &YIS) {
  IdHashNodeStableMapTy IdNodeStableMap;

  YIS >> IdNodeStableMap;
  YIS.nextDocument();

  convertFromStableData(IdNodeStableMap);
}

void OutlinedHashTreeRecord::convertToStableData(
    IdHashNodeStableMapTy &IdNodeStableMap) const {
  // Build NodeIdMap
  HashNodeIdMapTy NodeIdMap;
  HashTree->walkGraph(
      [&NodeIdMap](const HashNode *Current) {
        size_t Index = NodeIdMap.size();
        NodeIdMap[Current] = Index;
        assert((Index + 1 == NodeIdMap.size()) &&
               "Duplicate key in NodeIdMap: 'Current' should be unique.");
      },
      /*EdgeCallbackFn=*/nullptr, /*SortedWork=*/true);

  // Convert NodeIdMap to NodeStableMap
  for (auto &P : NodeIdMap) {
    auto *Node = P.first;
    auto Id = P.second;
    HashNodeStable NodeStable;
    NodeStable.Hash = Node->Hash;
    NodeStable.Terminals = Node->Terminals.value_or(0);
    for (auto &P : Node->Successors)
      NodeStable.SuccessorIds.push_back(NodeIdMap[P.second.get()]);
    IdNodeStableMap[Id] = NodeStable;
  }

  // Sort the Successors so that they come out in the same order as in the map.
  for (auto &P : IdNodeStableMap)
    toolchain::sort(P.second.SuccessorIds);
}

void OutlinedHashTreeRecord::convertFromStableData(
    const IdHashNodeStableMapTy &IdNodeStableMap) {
  IdHashNodeMapTy IdNodeMap;
  // Initialize the root node at 0.
  IdNodeMap[0] = HashTree->getRoot();
  assert(IdNodeMap[0]->Successors.empty());

  for (auto &P : IdNodeStableMap) {
    auto Id = P.first;
    const HashNodeStable &NodeStable = P.second;
    assert(IdNodeMap.count(Id));
    HashNode *Curr = IdNodeMap[Id];
    Curr->Hash = NodeStable.Hash;
    if (NodeStable.Terminals)
      Curr->Terminals = NodeStable.Terminals;
    auto &Successors = Curr->Successors;
    assert(Successors.empty());
    for (auto SuccessorId : NodeStable.SuccessorIds) {
      auto Sucessor = std::make_unique<HashNode>();
      IdNodeMap[SuccessorId] = Sucessor.get();
      auto Hash = IdNodeStableMap.at(SuccessorId).Hash;
      Successors[Hash] = std::move(Sucessor);
    }
  }
}
