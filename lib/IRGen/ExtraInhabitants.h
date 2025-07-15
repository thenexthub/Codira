//===--- ExtraInhabitants.h - Extra inhabitant routines ---------*- C++ -*-===//
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
// This file defines routines for working with extra inhabitants.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_EXTRAINHABITANTS_H
#define LANGUAGE_IRGEN_EXTRAINHABITANTS_H

#include "IRGen.h"

namespace toolchain {
class APInt;
class ConstantInt;
class Value;
}

namespace language {
namespace irgen {

class Address;
class Alignment;
class IRGenFunction;
class IRGenModule;

/// Whether the zero pointer is a valid value (i.e. not a valid
/// extra inhabitant) of a particular pointer type.
enum IsNullable_t: bool {
  IsNotNullable = false,
  IsNullable = true
};

/// Information about a particular pointer type.
struct PointerInfo {
  Alignment PointeeAlign;
  uint8_t NumReservedLowBits;
  IsNullable_t Nullable;

  static PointerInfo forHeapObject(const IRGenModule &IGM);
  static PointerInfo forFunction(const IRGenModule &IGM);
  static PointerInfo forAligned(Alignment pointeeAlign) {
    return {pointeeAlign, 0, IsNotNullable};
  }

  PointerInfo withNullable(IsNullable_t nullable) const {
    return {PointeeAlign, NumReservedLowBits, nullable};
  }

  /// Return the number of extra inhabitant representations for
  /// pointers with these properties: i.e. the number of values
  /// that do not collide with any valid pointers.
  unsigned getExtraInhabitantCount(const IRGenModule &IGM) const;


  /// Return an indexed extra inhabitant constant for a pointer
  /// with these properties.
  ///
  /// If the pointer appears within a larger aggregate, the
  /// 'bits' and 'offset' arguments can be used to position
  /// the inhabitant within the larger integer constant.
  toolchain::APInt getFixedExtraInhabitantValue(const IRGenModule &IGM,
                                           unsigned bits,
                                           unsigned index,
                                           unsigned offset) const;

  /// Given the address of storage for a pointer with these
  /// properties, return the extra inhabitant index of the
  /// value, or -1 if the value is a valid pointer.  Always
  /// produces an i32. 
  toolchain::Value *getExtraInhabitantIndex(IRGenFunction &IGF,
                                       Address src) const;

  /// Store an extra inhabitant representation for the given
  /// dynamic extra inhabitant index into the given storage.
  void storeExtraInhabitant(IRGenFunction &IGF,
                            toolchain::Value *index,
                            Address dest) const;
};

/*****************************************************************************/

/// \group Extra inhabitants of heap object pointers.

/// Return the number of extra inhabitant representations for heap objects,
/// that is, the number of invalid heap object pointer values that can be used
/// to represent enum tags for enums involving a reference type as a payload.
unsigned getHeapObjectExtraInhabitantCount(const IRGenModule &IGM);
  
/// Return an indexed extra inhabitant constant for a heap object pointer.
///
/// If the pointer appears within a larger aggregate, the 'bits' and 'offset'
/// arguments can be used to position the inhabitant within the larger integer
/// constant.
toolchain::APInt getHeapObjectFixedExtraInhabitantValue(const IRGenModule &IGM,
                                                   unsigned bits,
                                                   unsigned index,
                                                   unsigned offset);
  
/// Calculate the index of a heap object extra inhabitant representation stored
/// in memory.
toolchain::Value *getHeapObjectExtraInhabitantIndex(IRGenFunction &IGF,
                                               Address src);

/// Calculate an extra inhabitant representation from an index and store it to
/// memory.
void storeHeapObjectExtraInhabitant(IRGenFunction &IGF,
                                    toolchain::Value *index,
                                    Address dest);

} // end namespace irgen
} // end namespace language

#endif
