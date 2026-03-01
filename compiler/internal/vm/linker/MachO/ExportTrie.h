//===- ExportTrie.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLD_MACHO_EXPORT_TRIE_H
#define LLD_MACHO_EXPORT_TRIE_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"

#include <vector>

namespace lld::macho {

struct TrieNode;
class Symbol;

class TrieBuilder {
public:
  ~TrieBuilder();
  void setImageBase(uint64_t addr) { imageBase = addr; }
  void addSymbol(const Symbol &sym) { exported.push_back(&sym); }
  // Returns the size in bytes of the serialized trie.
  size_t build();
  void writeTo(uint8_t *buf) const;

private:
  TrieNode *makeNode();
  void sortAndBuild(llvm::MutableArrayRef<const Symbol *> vec, TrieNode *node,
                    size_t lastPos, size_t pos);

  uint64_t imageBase = 0;
  std::vector<const Symbol *> exported;
  std::vector<TrieNode *> nodes;
};

using TrieEntryCallback =
    llvm::function_ref<void(const llvm::Twine & /*name*/, uint64_t /*flags*/)>;

void parseTrie(const std::string &fileName, const uint8_t *buf, size_t size,
               const TrieEntryCallback &);

} // namespace lld::macho

#endif
