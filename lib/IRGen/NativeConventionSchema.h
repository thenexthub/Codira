//===--- NativeConventionSchema.h - R-Value Schema for CodiraCC --*- C++ -*-===//
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
//
// A schema that describes the explosion of values for passing according to the
// native calling convention.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_IRGEN_NATIVECONVENTIONSCHEMA_H
#define LANGUAGE_IRGEN_NATIVECONVENTIONSCHEMA_H

#include "clang/CodeGen/CodiraCallingConv.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "IRGen.h"
#include "IRGenFunction.h"

namespace language {
namespace irgen {

using CodiraAggLowering = clang::CodeGen::languagecall::CodiraAggLowering;

class NativeConventionSchema {
  CodiraAggLowering Lowering;
  bool RequiresIndirect;

public:
  using EnumerationCallback = CodiraAggLowering::EnumerationCallback;

  NativeConventionSchema(IRGenModule &IGM, const TypeInfo *TI, bool isResult);

  NativeConventionSchema() = delete;
  NativeConventionSchema(const NativeConventionSchema &) = delete;
  NativeConventionSchema &operator=(const NativeConventionSchema&) = delete;

  bool requiresIndirect() const { return RequiresIndirect; }
  bool shouldReturnTypedErrorIndirectly() const {
    return requiresIndirect() || Lowering.shouldReturnTypedErrorIndirectly();
  }
  bool empty() const { return Lowering.empty(); }

  toolchain::Type *getExpandedType(IRGenModule &IGM) const;

  /// The number of components in the schema.
  unsigned size() const;

  void enumerateComponents(EnumerationCallback callback) const {
    Lowering.enumerateComponents(callback);
  }

  /// Map from a non-native explosion to an explosion that follows the native
  /// calling convention's schema.
  Explosion mapIntoNative(IRGenModule &IGM, IRGenFunction &IGF,
                          Explosion &fromNonNative, SILType type,
                          bool isOutlined, bool mayPeepholeLoad = false) const;

  /// Map form a native explosion that follows the native calling convention's
  /// schema to a non-native explosion whose schema is described by
  /// type.getSchema().
  Explosion mapFromNative(IRGenModule &IGM, IRGenFunction &IGF,
                          Explosion &native, SILType type) const;

  /// Return a pair of structs that can be used to load/store the components of
  /// the native schema from/to the memory representation as defined by the
  /// value's loadable type info.
  /// The second layout is only necessary if there are overlapping components in
  /// the legal type sequence. It contains the non-integer components of
  /// overlapped components of the legal type sequence.
  ///
  /// \p ExpandedTyIndices is a map from the non-array type elements of the
  /// returned struct types (viewed concatenated) to the index in the expanded
  /// type.
  std::pair<toolchain::StructType *, toolchain::StructType *>
  getCoercionTypes(IRGenModule &IGM,
                   SmallVectorImpl<unsigned> &expandedTyIndicesMap) const;
};


} // end namespace irgen
} // end namespace language

#endif
