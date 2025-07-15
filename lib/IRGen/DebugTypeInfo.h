//===--- DebugTypeInfo.h - Type Info for Debugging --------------*- C++ -*-===//
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
// This file defines the data structure that holds all the debug info
// we want to emit for types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_DEBUGTYPEINFO_H
#define LANGUAGE_IRGEN_DEBUGTYPEINFO_H

#include "IRGen.h"
#include "language/AST/Decl.h"
#include "language/AST/Types.h"

namespace toolchain {
class Type;
}

namespace language {
class SILDebugScope;
class SILGlobalVariable;

namespace irgen {
class TypeInfo;
class IRGenModule;

/// This data structure holds everything needed to emit debug info
/// for a type.
class DebugTypeInfo {
protected:
  /// The type we need to emit may be different from the type
  /// mentioned in the Decl, for example, stripped of qualifiers.
  TypeBase *Type = nullptr;
  /// Needed to determine the size of basic types and to determine
  /// the storage type for undefined variables.
  std::optional<uint32_t> NumExtraInhabitants;
  Alignment Align;
  bool DefaultAlignment = true;
  bool IsMetadataType = false;
  bool IsFixedBuffer = false;
  bool IsForwardDecl = false;

public:
  DebugTypeInfo() = default;
  DebugTypeInfo(language::Type Ty, Alignment AlignInBytes = Alignment(1),
                bool HasDefaultAlignment = true, bool IsMetadataType = false,
                bool IsFixedBuffer = false,
                std::optional<uint32_t> NumExtraInhabitants = {});

  /// Create type for a local variable.
  static DebugTypeInfo getLocalVariable(VarDecl *Decl, language::Type Ty,
                                        const TypeInfo &Info, IRGenModule &IGM);
  /// Create type for global type metadata.
  static DebugTypeInfo getGlobalMetadata(language::Type Ty, Size size,
                                         Alignment align);
  /// Create type for an artificial metadata variable.
  static DebugTypeInfo getTypeMetadata(language::Type Ty, Size size,
                                       Alignment align);

  /// Create a forward declaration for a type whose size is unknown.
  static DebugTypeInfo getForwardDecl(language::Type Ty);

  /// Create a standalone type from a TypeInfo object.
  static DebugTypeInfo getFromTypeInfo(language::Type Ty, const TypeInfo &Info,
                                       IRGenModule &IGM);
  /// Global variables.
  static DebugTypeInfo getGlobal(SILGlobalVariable *GV, IRGenModule &IGM);
  static DebugTypeInfo getGlobalFixedBuffer(SILGlobalVariable *GV,
                                            Alignment align, IRGenModule &IGM);
  /// ObjC classes.
  static DebugTypeInfo getObjCClass(ClassDecl *theClass, Size size,
                                    Alignment align);
  /// Error type.
  static DebugTypeInfo getErrorResult(language::Type Ty, IRGenModule &IGM);

  TypeBase *getType() const { return Type; }

  TypeDecl *getDecl() const;

  // Determine whether this type is an Archetype dependent on a generic context.
  bool isContextArchetype() const {
    if (auto archetype =
            Type->getWithoutSpecifierType()->getAs<ArchetypeType>()) {
      return !isa<OpaqueTypeArchetypeType>(archetype);
    }
    return false;
  }

  Alignment getAlignment() const { return Align; }
  bool isNull() const { return !Type; }
  bool isMetadataType() const { return IsMetadataType; }
  bool hasDefaultAlignment() const { return DefaultAlignment; }
  bool isFixedBuffer() const { return IsFixedBuffer; }
  bool isForwardDecl() const { return IsForwardDecl; }
  std::optional<uint32_t> getNumExtraInhabitants() const {
    return NumExtraInhabitants;
  }

  bool operator==(DebugTypeInfo T) const;
  bool operator!=(DebugTypeInfo T) const;
#if !defined(NDEBUG) || defined(TOOLCHAIN_ENABLE_DUMP)
  TOOLCHAIN_DUMP_METHOD void dump() const;
#endif
};

/// A DebugTypeInfo with a defined size (that may be 0).
class CompletedDebugTypeInfo : public DebugTypeInfo {
  Size::int_type SizeInBits;

  CompletedDebugTypeInfo(DebugTypeInfo DbgTy, Size::int_type SizeInBits)
    : DebugTypeInfo(DbgTy), SizeInBits(SizeInBits) {}

public:
  static std::optional<CompletedDebugTypeInfo>
  get(DebugTypeInfo DbgTy, std::optional<Size::int_type> SizeInBits) {
    if (!SizeInBits)
      return {};
    return CompletedDebugTypeInfo(DbgTy, *SizeInBits);
  }

  static std::optional<CompletedDebugTypeInfo>
  getFromTypeInfo(language::Type Ty, const TypeInfo &Info, IRGenModule &IGM,
                  std::optional<Size::int_type> SizeInBits = {});

  Size::int_type getSizeInBits() const { return SizeInBits; }
};

}
}

namespace toolchain {

// Dense map specialization.
template <> struct DenseMapInfo<language::irgen::DebugTypeInfo> {
  static language::irgen::DebugTypeInfo getEmptyKey() {
    return {};
  }
  static language::irgen::DebugTypeInfo getTombstoneKey() {
    return language::irgen::DebugTypeInfo(
        toolchain::DenseMapInfo<language::TypeBase *>::getTombstoneKey(),
        language::irgen::Alignment(), /* HasDefaultAlignment = */ false);
  }
  static unsigned getHashValue(language::irgen::DebugTypeInfo Val) {
    return DenseMapInfo<language::CanType>::getHashValue(Val.getType());
  }
  static bool isEqual(language::irgen::DebugTypeInfo LHS,
                      language::irgen::DebugTypeInfo RHS) {
    return LHS == RHS;
  }
};
}

#endif
