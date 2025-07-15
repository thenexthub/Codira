//===--- ReferenceTypeInfo.h - Supplement for reference types ---*- C++ -*-===//
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
// This file defines ReferenceTypeInfo, which supplements the
// FixedTypeInfo interface for types with reference semantics.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_REFERENCETYPEINFO_H
#define LANGUAGE_IRGEN_REFERENCETYPEINFO_H

#include "LoadableTypeInfo.h"

namespace language {
namespace irgen {

class TypeConverter;
  
/// An abstract class designed for use when implementing a type
/// that has reference semantics.
class ReferenceTypeInfo : public LoadableTypeInfo {
protected:
  // FIXME: Get spare bits for pointers from a TargetInfo-like structure.
  ReferenceTypeInfo(toolchain::Type *type, Size size, SpareBitVector spareBits,
                    Alignment align, IsTriviallyDestroyable_t pod = IsNotTriviallyDestroyable)
    : LoadableTypeInfo(type, size, spareBits, align, pod, IsCopyable,
                       IsFixedSize, IsABIAccessible, SpecialTypeInfoKind::Reference)
  {}

public:
  /// Strongly retains a value.
  virtual void strongRetain(IRGenFunction &IGF, Explosion &in,
                            Atomicity atomicity) const = 0;

  /// Strongly releases a value.
  virtual void strongRelease(IRGenFunction &IGF, Explosion &in,
                             Atomicity atomicity) const = 0;

  virtual ReferenceCounting getReferenceCountingType() const {
    toolchain_unreachable("not supported");
  }

#define REF_STORAGE_HELPER(Name) \
  virtual const TypeInfo *create##Name##StorageType(TypeConverter &TC, \
                                                    bool isOptional) const = 0;
#define NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  virtual void name##TakeStrong(IRGenFunction &IGF, Address addr, \
                                 Explosion &out, bool isOptional) const = 0; \
  virtual void name##LoadStrong(IRGenFunction &IGF, Address addr, \
                                 Explosion &out, bool isOptional) const = 0; \
  virtual void name##Init(IRGenFunction &IGF, Explosion &in, \
                           Address dest, bool isOptional) const = 0; \
  virtual void name##Assign(IRGenFunction &IGF, Explosion &in, \
                             Address dest, bool isOptional) const = 0;
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  virtual void strongRetain##Name(IRGenFunction &IGF, Explosion &in, \
                                  Atomicity atomicity) const = 0; \
  virtual void strongRetain##Name##Release(IRGenFunction &IGF, \
                                           Explosion &in, \
                                           Atomicity atomicity) const = 0; \
  virtual void name##Retain(IRGenFunction &IGF, Explosion &in, \
                             Atomicity atomicity) const = 0; \
  virtual void name##Release(IRGenFunction &IGF, Explosion &in, \
                              Atomicity atomicity) const = 0;
#define NEVER_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  REF_STORAGE_HELPER(Name)
#define ALWAYS_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  REF_STORAGE_HELPER(Name)
#define SOMETIMES_LOADABLE_CHECKED_REF_STORAGE(Name, name, ...) \
  NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER(Name, name) \
  REF_STORAGE_HELPER(Name)
#define UNCHECKED_REF_STORAGE(Name, name, ...) \
  REF_STORAGE_HELPER(Name)
#include "language/AST/ReferenceStorage.def"
#undef REF_STORAGE_HELPER
#undef NEVER_LOADABLE_CHECKED_REF_STORAGE_HELPER
#undef ALWAYS_LOADABLE_CHECKED_REF_STORAGE_HELPER

  static bool classof(const ReferenceTypeInfo *type) { return true; }
  static bool classof(const TypeInfo *type) {
    return type->getSpecialTypeInfoKind() == SpecialTypeInfoKind::Reference;
  }
};

}
}

#endif
