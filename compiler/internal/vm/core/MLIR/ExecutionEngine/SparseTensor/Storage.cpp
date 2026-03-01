//===- StorageBase.cpp - TACO-flavored sparse tensor representation -------===//
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
// This file contains method definitions for `SparseTensorStorageBase`.
// In particular we want to ensure that the default implementations of
// the "partial method specialization" trick aren't inline (since there's
// no benefit).
//
//===----------------------------------------------------------------------===//

#include "mlir/ExecutionEngine/SparseTensor/Storage.h"

using namespace mlir::sparse_tensor;

static inline bool isAllDense(uint64_t lvlRank, const LevelType *lvlTypes) {
  for (uint64_t l = 0; l < lvlRank; l++)
    if (!isDenseLT(lvlTypes[l]))
      return false;
  return true;
}

SparseTensorStorageBase::SparseTensorStorageBase( // NOLINT
    uint64_t dimRank, const uint64_t *dimSizes, uint64_t lvlRank,
    const uint64_t *lvlSizes, const LevelType *lvlTypes,
    const uint64_t *dim2lvl, const uint64_t *lvl2dim)
    : dimSizes(dimSizes, dimSizes + dimRank),
      lvlSizes(lvlSizes, lvlSizes + lvlRank),
      lvlTypes(lvlTypes, lvlTypes + lvlRank),
      dim2lvlVec(dim2lvl, dim2lvl + lvlRank),
      lvl2dimVec(lvl2dim, lvl2dim + dimRank),
      map(dimRank, lvlRank, dim2lvlVec.data(), lvl2dimVec.data()),
      allDense(isAllDense(lvlRank, lvlTypes)) {
  assert(dimSizes && lvlSizes && lvlTypes && dim2lvl && lvl2dim);
  // Validate dim-indexed parameters.
  assert(dimRank > 0 && "Trivial shape is unsupported");
  for (uint64_t d = 0; d < dimRank; d++)
    assert(dimSizes[d] > 0 && "Dimension size zero has trivial storage");
  // Validate lvl-indexed parameters.
  assert(lvlRank > 0 && "Trivial shape is unsupported");
  for (uint64_t l = 0; l < lvlRank; l++) {
    assert(lvlSizes[l] > 0 && "Level size zero has trivial storage");
    assert(isDenseLvl(l) || isCompressedLvl(l) || isLooseCompressedLvl(l) ||
           isSingletonLvl(l) || isNOutOfMLvl(l));
  }
}

// Helper macro for wrong "partial method specialization" errors.
#define FATAL_PIV(NAME)                                                        \
  fprintf(stderr, "<P,I,V> type mismatch for: " #NAME);                        \
  exit(1);

#define IMPL_GETPOSITIONS(PNAME, P)                                            \
  void SparseTensorStorageBase::getPositions(std::vector<P> **, uint64_t) {    \
    FATAL_PIV("getPositions" #PNAME);                                          \
  }
MLIR_SPARSETENSOR_FOREVERY_FIXED_O(IMPL_GETPOSITIONS)
#undef IMPL_GETPOSITIONS

#define IMPL_GETCOORDINATES(CNAME, C)                                          \
  void SparseTensorStorageBase::getCoordinates(std::vector<C> **, uint64_t) {  \
    FATAL_PIV("getCoordinates" #CNAME);                                        \
  }
MLIR_SPARSETENSOR_FOREVERY_FIXED_O(IMPL_GETCOORDINATES)
#undef IMPL_GETCOORDINATES

#define IMPL_GETCOORDINATESBUFFER(CNAME, C)                                    \
  void SparseTensorStorageBase::getCoordinatesBuffer(std::vector<C> **,        \
                                                     uint64_t) {               \
    FATAL_PIV("getCoordinatesBuffer" #CNAME);                                  \
  }
MLIR_SPARSETENSOR_FOREVERY_FIXED_O(IMPL_GETCOORDINATESBUFFER)
#undef IMPL_GETCOORDINATESBUFFER

#define IMPL_GETVALUES(VNAME, V)                                               \
  void SparseTensorStorageBase::getValues(std::vector<V> **) {                 \
    FATAL_PIV("getValues" #VNAME);                                             \
  }
MLIR_SPARSETENSOR_FOREVERY_V(IMPL_GETVALUES)
#undef IMPL_GETVALUES

#define IMPL_LEXINSERT(VNAME, V)                                               \
  void SparseTensorStorageBase::lexInsert(const uint64_t *, V) {               \
    FATAL_PIV("lexInsert" #VNAME);                                             \
  }
MLIR_SPARSETENSOR_FOREVERY_V(IMPL_LEXINSERT)
#undef IMPL_LEXINSERT

#define IMPL_EXPINSERT(VNAME, V)                                               \
  void SparseTensorStorageBase::expInsert(uint64_t *, V *, bool *, uint64_t *, \
                                          uint64_t, uint64_t) {                \
    FATAL_PIV("expInsert" #VNAME);                                             \
  }
MLIR_SPARSETENSOR_FOREVERY_V(IMPL_EXPINSERT)
#undef IMPL_EXPINSERT

#undef FATAL_PIV
