//===----------------------------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file contains implementation details, such as storage structures, of
// CIR dialect types.
//
//===----------------------------------------------------------------------===//
#ifndef CIR_DIALECT_IR_CIRTYPESDETAILS_H
#define CIR_DIALECT_IR_CIRTYPESDETAILS_H

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/Support/LogicalResult.h"
#include "language/Core/CIR/Dialect/IR/CIRTypes.h"
#include "toolchain/ADT/Hashing.h"

namespace cir {
namespace detail {

//===----------------------------------------------------------------------===//
// CIR RecordTypeStorage
//===----------------------------------------------------------------------===//

/// Type storage for CIR record types.
struct RecordTypeStorage : public mlir::TypeStorage {
  struct KeyTy {
    toolchain::ArrayRef<mlir::Type> members;
    mlir::StringAttr name;
    bool incomplete;
    bool packed;
    bool padded;
    RecordType::RecordKind kind;

    KeyTy(toolchain::ArrayRef<mlir::Type> members, mlir::StringAttr name,
          bool incomplete, bool packed, bool padded,
          RecordType::RecordKind kind)
        : members(members), name(name), incomplete(incomplete), packed(packed),
          padded(padded), kind(kind) {}
  };

  toolchain::ArrayRef<mlir::Type> members;
  mlir::StringAttr name;
  bool incomplete;
  bool packed;
  bool padded;
  RecordType::RecordKind kind;

  RecordTypeStorage(toolchain::ArrayRef<mlir::Type> members, mlir::StringAttr name,
                    bool incomplete, bool packed, bool padded,
                    RecordType::RecordKind kind)
      : members(members), name(name), incomplete(incomplete), packed(packed),
        padded(padded), kind(kind) {
    assert((name || !incomplete) && "Incomplete records must have a name");
  }

  KeyTy getAsKey() const {
    return KeyTy(members, name, incomplete, packed, padded, kind);
  }

  bool operator==(const KeyTy &key) const {
    if (name)
      return (name == key.name) && (kind == key.kind);
    return std::tie(members, name, incomplete, packed, padded, kind) ==
           std::tie(key.members, key.name, key.incomplete, key.packed,
                    key.padded, key.kind);
  }

  static toolchain::hash_code hashKey(const KeyTy &key) {
    if (key.name)
      return toolchain::hash_combine(key.name, key.kind);
    return toolchain::hash_combine(key.members, key.incomplete, key.packed,
                              key.padded, key.kind);
  }

  static RecordTypeStorage *construct(mlir::TypeStorageAllocator &allocator,
                                      const KeyTy &key) {
    return new (allocator.allocate<RecordTypeStorage>())
        RecordTypeStorage(allocator.copyInto(key.members), key.name,
                          key.incomplete, key.packed, key.padded, key.kind);
  }

  /// Mutates the members and attributes an identified record.
  ///
  /// Once a record is mutated, it is marked as complete, preventing further
  /// mutations. Anonymous records are always complete and cannot be mutated.
  /// This method does not fail if a mutation of a complete record does not
  /// change the record.
  toolchain::LogicalResult mutate(mlir::TypeStorageAllocator &allocator,
                             toolchain::ArrayRef<mlir::Type> members, bool packed,
                             bool padded) {
    // Anonymous records cannot mutate.
    if (!name)
      return toolchain::failure();

    // Mutation of complete records are allowed if they change nothing.
    if (!incomplete)
      return mlir::success((this->members == members) &&
                           (this->packed == packed) &&
                           (this->padded == padded));

    // Mutate incomplete record.
    this->members = allocator.copyInto(members);
    this->packed = packed;
    this->padded = padded;

    incomplete = false;
    return toolchain::success();
  }
};

} // namespace detail
} // namespace cir

#endif // CIR_DIALECT_IR_CIRTYPESDETAILS_H
