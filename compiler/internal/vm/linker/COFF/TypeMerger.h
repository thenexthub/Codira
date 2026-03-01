//===- TypeMerger.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLD_COFF_TYPEMERGER_H
#define LLD_COFF_TYPEMERGER_H

#include "COFFLinkerContext.h"
#include "Config.h"
#include "DebugTypes.h"
#include "lld/Common/Timer.h"
#include "llvm/DebugInfo/CodeView/MergingTypeTableBuilder.h"
#include "llvm/DebugInfo/CodeView/TypeHashing.h"
#include "llvm/Support/Allocator.h"
#include <atomic>

namespace lld::coff {

using llvm::codeview::GloballyHashedType;
using llvm::codeview::TypeIndex;

struct GHashState;

class TypeMerger {
public:
  TypeMerger(COFFLinkerContext &ctx, llvm::BumpPtrAllocator &alloc);

  ~TypeMerger();

  /// Get the type table or the global type table if /DEBUG:GHASH is enabled.
  inline llvm::codeview::TypeCollection &getTypeTable() {
    assert(!ctx.config.debugGHashes);
    return typeTable;
  }

  /// Get the ID table or the global ID table if /DEBUG:GHASH is enabled.
  inline llvm::codeview::TypeCollection &getIDTable() {
    assert(!ctx.config.debugGHashes);
    return idTable;
  }

  /// Use global hashes to eliminate duplicate types and identify unique type
  /// indices in each TpiSource.
  void mergeTypesWithGHash();

  /// Map from PDB function id type indexes to PDB function type indexes.
  /// Populated after mergeTypesWithGHash.
  llvm::DenseMap<TypeIndex, TypeIndex> funcIdToType;

  /// Type records that will go into the PDB TPI stream.
  llvm::codeview::MergingTypeTableBuilder typeTable;

  /// Item records that will go into the PDB IPI stream.
  llvm::codeview::MergingTypeTableBuilder idTable;

  // When showSummary is enabled, these are histograms of TPI and IPI records
  // keyed by type index.
  SmallVector<uint32_t, 0> tpiCounts;
  SmallVector<uint32_t, 0> ipiCounts;

  /// Dependency type sources, such as type servers or PCH object files. These
  /// must be processed before objects that rely on them. Set by
  /// sortDependencies.
  ArrayRef<TpiSource *> dependencySources;

  /// Object file sources. These must be processed after dependencySources.
  ArrayRef<TpiSource *> objectSources;

  /// Sorts the dependencies and reassigns TpiSource indices.
  void sortDependencies();

private:
  void clearGHashes();

  COFFLinkerContext &ctx;
};

} // namespace lld::coff

#endif
