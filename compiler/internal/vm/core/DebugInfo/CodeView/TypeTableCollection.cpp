//===- TypeTableCollection.cpp -------------------------------- *- C++ --*-===//
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

#include "vm/core/DebugInfo/CodeView/TypeTableCollection.h"

#include "vm/core/DebugInfo/CodeView/RecordName.h"
#include "vm/core/DebugInfo/CodeView/TypeIndex.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;
using namespace vm::core::codeview;

TypeTableCollection::TypeTableCollection(ArrayRef<ArrayRef<uint8_t>> Records)
    : NameStorage(Allocator), Records(Records) {
  Names.resize(Records.size());
}

std::optional<TypeIndex> TypeTableCollection::getFirst() {
  if (empty())
    return std::nullopt;
  return TypeIndex::fromArrayIndex(0);
}

std::optional<TypeIndex> TypeTableCollection::getNext(TypeIndex Prev) {
  assert(contains(Prev));
  ++Prev;
  if (Prev.toArrayIndex() == size())
    return std::nullopt;
  return Prev;
}

CVType TypeTableCollection::getType(TypeIndex Index) {
  assert(Index.toArrayIndex() < Records.size());
  return CVType(Records[Index.toArrayIndex()]);
}

StringRef TypeTableCollection::getTypeName(TypeIndex Index) {
  if (Index.isNoneType() || Index.isSimple())
    return TypeIndex::simpleTypeName(Index);

  uint32_t I = Index.toArrayIndex();
  if (Names[I].data() == nullptr) {
    StringRef Result = NameStorage.save(computeTypeName(*this, Index));
    Names[I] = Result;
  }
  return Names[I];
}

bool TypeTableCollection::contains(TypeIndex Index) {
  return Index.toArrayIndex() <= size();
}

uint32_t TypeTableCollection::size() { return Records.size(); }

uint32_t TypeTableCollection::capacity() { return Records.size(); }

bool TypeTableCollection::replaceType(TypeIndex &Index, CVType Data,
                                      bool Stabilize) {
  llvm_unreachable("Method cannot be called");
}
