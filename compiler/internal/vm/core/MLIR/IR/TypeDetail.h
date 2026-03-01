//===- TypeDetail.h - MLIR Type storage details -----------------*- C++ -*-===//
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
// This holds implementation details of Type.
//
//===----------------------------------------------------------------------===//
#ifndef TYPEDETAIL_H_
#define TYPEDETAIL_H_

#include "mlir/IR/AffineMap.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/OperationSupport.h"
#include "mlir/IR/TypeRange.h"
#include "vm/core/ADT/bit.h"
#include "vm/core/Support/TrailingObjects.h"

namespace mlir {

namespace detail {

/// Integer Type Storage and Uniquing.
struct IntegerTypeStorage : public TypeStorage {
  IntegerTypeStorage(unsigned width,
                     IntegerType::SignednessSemantics signedness)
      : width(width), signedness(signedness) {}

  /// The hash key used for uniquing.
  using KeyTy = std::tuple<unsigned, IntegerType::SignednessSemantics>;

  static toolchain::hash_code hashKey(const KeyTy &key) {
    return toolchain::hash_value(key);
  }

  bool operator==(const KeyTy &key) const {
    return KeyTy(width, signedness) == key;
  }

  static IntegerTypeStorage *construct(TypeStorageAllocator &allocator,
                                       KeyTy key) {
    return new (allocator.allocate<IntegerTypeStorage>())
        IntegerTypeStorage(std::get<0>(key), std::get<1>(key));
  }

  KeyTy getAsKey() const { return KeyTy(width, signedness); }

  unsigned width : 30;
  IntegerType::SignednessSemantics signedness : 2;
};

/// Function Type Storage and Uniquing.
struct FunctionTypeStorage : public TypeStorage {
  FunctionTypeStorage(unsigned numInputs, unsigned numResults,
                      Type const *inputsAndResults)
      : numInputs(numInputs), numResults(numResults),
        inputsAndResults(inputsAndResults) {}

  /// The hash key used for uniquing.
  using KeyTy = std::tuple<TypeRange, TypeRange>;
  bool operator==(const KeyTy &key) const {
    if (std::get<0>(key) == getInputs())
      return std::get<1>(key) == getResults();
    return false;
  }

  /// Construction.
  static FunctionTypeStorage *construct(TypeStorageAllocator &allocator,
                                        const KeyTy &key) {
    auto [inputs, results] = key;

    // Copy the inputs and results into the bump pointer.
    SmallVector<Type, 16> types;
    types.reserve(inputs.size() + results.size());
    types.append(inputs.begin(), inputs.end());
    types.append(results.begin(), results.end());
    auto typesList = allocator.copyInto(ArrayRef<Type>(types));

    // Initialize the memory using placement new.
    return new (allocator.allocate<FunctionTypeStorage>())
        FunctionTypeStorage(inputs.size(), results.size(), typesList.data());
  }

  ArrayRef<Type> getInputs() const {
    return ArrayRef<Type>(inputsAndResults, numInputs);
  }
  ArrayRef<Type> getResults() const {
    return ArrayRef<Type>(inputsAndResults + numInputs, numResults);
  }

  KeyTy getAsKey() const { return KeyTy(getInputs(), getResults()); }

  unsigned numInputs;
  unsigned numResults;
  Type const *inputsAndResults;
};

/// A type representing a collection of other types.
struct TupleTypeStorage final
    : public TypeStorage,
      private toolchain::TrailingObjects<TupleTypeStorage, Type> {
  friend toolchain::TrailingObjects<TupleTypeStorage, Type>;
  using KeyTy = TypeRange;

  TupleTypeStorage(unsigned numTypes) : numElements(numTypes) {}

  /// Construction.
  static TupleTypeStorage *construct(TypeStorageAllocator &allocator,
                                     TypeRange key) {
    // Allocate a new storage instance.
    auto byteSize = TupleTypeStorage::totalSizeToAlloc<Type>(key.size());
    auto *rawMem = allocator.allocate(byteSize, alignof(TupleTypeStorage));
    auto *result = ::new (rawMem) TupleTypeStorage(key.size());

    // Copy in the element types into the trailing storage.
    toolchain::uninitialized_copy(key, result->getTrailingObjects());
    return result;
  }

  bool operator==(const KeyTy &key) const { return key == getTypes(); }

  /// Return the number of held types.
  unsigned size() const { return numElements; }

  /// Return the held types.
  ArrayRef<Type> getTypes() const { return getTrailingObjects(size()); }

  KeyTy getAsKey() const { return getTypes(); }

  /// The number of tuple elements.
  unsigned numElements;
};

/// Checks if the memorySpace has supported Attribute type.
bool isSupportedMemorySpace(Attribute memorySpace);

/// Wraps deprecated integer memory space to the new Attribute form.
Attribute wrapIntegerMemorySpace(unsigned memorySpace, MLIRContext *ctx);

/// Replaces default memorySpace (integer == `0`) with empty Attribute.
Attribute skipDefaultMemorySpace(Attribute memorySpace);

/// [deprecated] Returns the memory space in old raw integer representation.
/// New `Attribute getMemorySpace()` method should be used instead.
unsigned getMemorySpaceAsInt(Attribute memorySpace);

} // namespace detail
} // namespace mlir

#endif // TYPEDETAIL_H_
