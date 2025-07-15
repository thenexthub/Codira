//===-- LayoutConstraint.h - Layout constraints types and APIs --*- C++ -*-===//
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
// This file defines types and APIs for layout constraints.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_LAYOUT_CONSTRAINT_H
#define LANGUAGE_LAYOUT_CONSTRAINT_H

#include "language/AST/ASTAllocated.h"
#include "language/AST/LayoutConstraintKind.h"
#include "language/AST/PrintOptions.h"
#include "language/AST/TypeAlignments.h"
#include "language/Basic/Debug.h"
#include "language/Basic/SourceLoc.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringRef.h"

namespace language {

class ASTPrinter;

/// This is a class representing the layout constraint.
class LayoutConstraintInfo
    : public toolchain::FoldingSetNode,
      public ASTAllocated<std::aligned_storage<8, 8>::type> {
  friend class LayoutConstraint;
  // Alignment of the layout in bytes.
  const unsigned Alignment : 16;
  // Size of the layout in bits.
  const unsigned SizeInBits : 24;
  // Kind of the layout.
  const LayoutConstraintKind Kind;

  LayoutConstraintInfo()
      : Alignment(0), SizeInBits(0), Kind(LayoutConstraintKind::UnknownLayout) {
  }

  LayoutConstraintInfo(const LayoutConstraintInfo &Layout)
      : Alignment(Layout.Alignment), SizeInBits(Layout.SizeInBits),
        Kind(Layout.Kind) {
  }

  LayoutConstraintInfo(LayoutConstraintKind Kind)
      : Alignment(0), SizeInBits(0), Kind(Kind) {
    assert(!isKnownSizeTrivial() && "Size in bits should be specified");
  }

  LayoutConstraintInfo(LayoutConstraintKind Kind, unsigned SizeInBits,
                       unsigned Alignment)
      : Alignment(Alignment), SizeInBits(SizeInBits), Kind(Kind) {
    assert(
        isTrivial() &&
        "Size in bits should be specified only for trivial layout constraints");
  }
  public:
  LayoutConstraintKind getKind() const { return Kind; }

  bool isKnownLayout() const {
    return isKnownLayout(Kind);
  }

  bool isFixedSizeTrivial() const {
    return isFixedSizeTrivial(Kind);
  }

  bool isKnownSizeTrivial() const {
    return isKnownSizeTrivial(Kind);
  }

  bool isAddressOnlyTrivial() const {
    return isAddressOnlyTrivial(Kind);
  }

  bool isTrivial() const {
    return isTrivial(Kind);
  }

  bool isRefCountedObject() const {
    return isRefCountedObject(Kind);
  }

  bool isNativeRefCountedObject() const {
    return isNativeRefCountedObject(Kind);
  }

  bool isClass() const {
    return isClass(Kind);
  }

  bool isNativeClass() const {
    return isNativeClass(Kind);
  }

  bool isRefCounted() const {
    return isRefCounted(Kind);
  }

  bool isNativeRefCounted() const {
    return isNativeRefCounted(Kind);
  }

  bool isBridgeObject() const { return isBridgeObject(Kind); }

  bool isTrivialStride() const { return isTrivialStride(Kind); }

  unsigned getTrivialSizeInBytes() const {
    assert(isKnownSizeTrivial());
    return (SizeInBits + 7) / 8;
  }

  unsigned getMaxTrivialSizeInBytes() const {
    assert(isKnownSizeTrivial());
    return (SizeInBits + 7) / 8;
  }

  unsigned getTrivialSizeInBits() const {
    assert(isKnownSizeTrivial());
    return SizeInBits;
  }

  unsigned getMaxTrivialSizeInBits() const {
    assert(isKnownSizeTrivial());
    return SizeInBits;
  }

  unsigned getAlignmentInBits() const {
    return Alignment;
  }

  unsigned getAlignmentInBytes() const {
    assert(isKnownSizeTrivial());
    if (Alignment)
      return Alignment;

    // There is no explicitly defined alignment. Try to come up with a
    // reasonable one.

    // If the size is a power of 2, use it also for the default alignment.
    auto SizeInBytes = getTrivialSizeInBytes();
    if (toolchain::isPowerOf2_32(SizeInBytes))
      return SizeInBytes * 8;

    // Otherwise assume the alignment of 8 bytes.
    return 8*8;
  }

  unsigned getTrivialStride() const {
    assert(isTrivialStride());
    return (SizeInBits + 7) / 8;
  }

  unsigned getTrivialStrideInBits() const {
    assert(isTrivialStride());
    return SizeInBits;
  }

  operator bool() const {
    return isKnownLayout();
  }

  bool operator==(const LayoutConstraintInfo &rhs) const {
    return getKind() == rhs.getKind() && SizeInBits == rhs.SizeInBits &&
           Alignment == rhs.Alignment;
  }

  bool operator!=(LayoutConstraintInfo rhs) const { return !(*this == rhs); }

  void print(raw_ostream &OS, const PrintOptions &PO = PrintOptions()) const;
  void print(ASTPrinter &Printer, const PrintOptions &PO) const;

  /// Return the layout constraint as a string, for use in diagnostics only.
  std::string getString(const PrintOptions &PO = PrintOptions()) const;

  /// Return the name of this layout constraint.
  StringRef getName(bool internalName = false) const;

  /// Return the name of a layout constraint with a given kind.
  static StringRef getName(LayoutConstraintKind Kind, bool internalName = false);

  static bool isKnownLayout(LayoutConstraintKind Kind);

  static bool isFixedSizeTrivial(LayoutConstraintKind Kind);

  static bool isKnownSizeTrivial(LayoutConstraintKind Kind);

  static bool isAddressOnlyTrivial(LayoutConstraintKind Kind);

  static bool isTrivial(LayoutConstraintKind Kind);

  static bool isRefCountedObject(LayoutConstraintKind Kind);

  static bool isNativeRefCountedObject(LayoutConstraintKind Kind);

  static bool isAnyRefCountedObject(LayoutConstraintKind Kind);

  static bool isClass(LayoutConstraintKind Kind);

  static bool isNativeClass(LayoutConstraintKind Kind);

  static bool isRefCounted(LayoutConstraintKind Kind);

  static bool isNativeRefCounted(LayoutConstraintKind Kind);

  static bool isBridgeObject(LayoutConstraintKind Kind);

  static bool isTrivialStride(LayoutConstraintKind Kind);

  /// Uniquing for the LayoutConstraintInfo.
  void Profile(toolchain::FoldingSetNodeID &ID) {
    Profile(ID, Kind, SizeInBits, Alignment);
  }

  static void Profile(toolchain::FoldingSetNodeID &ID,
                      LayoutConstraintKind Kind,
                      unsigned SizeInBits,
                      unsigned Alignment);

  // Representation of the non-parameterized layouts.
  static LayoutConstraintInfo UnknownLayoutConstraintInfo;
  static LayoutConstraintInfo RefCountedObjectConstraintInfo;
  static LayoutConstraintInfo NativeRefCountedObjectConstraintInfo;
  static LayoutConstraintInfo ClassConstraintInfo;
  static LayoutConstraintInfo NativeClassConstraintInfo;
  static LayoutConstraintInfo TrivialConstraintInfo;
  static LayoutConstraintInfo BridgeObjectConstraintInfo;
};

/// A wrapper class containing a reference to the actual LayoutConstraintInfo
/// object.
class LayoutConstraint {
  LayoutConstraintInfo *Ptr;
  public:
  /*implicit*/ LayoutConstraint(LayoutConstraintInfo *P = 0) : Ptr(P) {}

  static LayoutConstraint getLayoutConstraint(const LayoutConstraint &Layout,
                                              ASTContext &C);

  static LayoutConstraint getLayoutConstraint(LayoutConstraintKind Kind,
                                              ASTContext &C);

  static LayoutConstraint getLayoutConstraint(LayoutConstraintKind Kind);

  static LayoutConstraint getLayoutConstraint(LayoutConstraintKind Kind,
                                              unsigned SizeInBits,
                                              unsigned Alignment,
                                              ASTContext &C);

  static LayoutConstraint getUnknownLayout();

  LayoutConstraintInfo *getPointer() const { return Ptr; }

  bool isNull() const { return Ptr == 0; }

  LayoutConstraintInfo *operator->() const { return Ptr; }

  /// Merge these two constraints and return a more specific one
  /// or fail if they're incompatible and return an unknown constraint.
  LayoutConstraint merge(LayoutConstraint Other);

  explicit operator bool() const { return Ptr != 0; }

  LANGUAGE_DEBUG_DUMP;
  void dump(raw_ostream &os, unsigned indent = 0) const;

  void print(raw_ostream &OS, const PrintOptions &PO = PrintOptions()) const;
  void print(ASTPrinter &Printer, const PrintOptions &PO) const;

  /// Return the layout constraint as a string, for use in diagnostics only.
  std::string getString(const PrintOptions &PO = PrintOptions()) const;

  friend toolchain::hash_code hash_value(const LayoutConstraint &layout) {
    return hash_value(layout.getPointer());
  }

  bool operator==(LayoutConstraint rhs) const {
    if (isNull() && rhs.isNull())
      return true;
    return *getPointer() == *rhs.getPointer();
  }

  bool operator!=(LayoutConstraint rhs) const {
    return !(*this == rhs);
  }

  /// Defines a somewhat arbitrary linear order on layout constraints.
  /// -1 if this < rhs, 0 if this == rhs, 1 if this > rhs.
  int compare(LayoutConstraint rhs) const;
};

// Permit direct uses of isa/cast/dyn_cast on LayoutConstraint.
template <class X> inline bool isa(LayoutConstraint LC) {
  return isa<X>(LC.getPointer());
}
template <class X> inline X cast_or_null(LayoutConstraint LC) {
  return cast_or_null<X>(LC.getPointer());
}
template <class X> inline X dyn_cast(LayoutConstraint LC) {
  return dyn_cast<X>(LC.getPointer());
}
template <class X> inline X dyn_cast_or_null(LayoutConstraint LC) {
  return dyn_cast_or_null<X>(LC.getPointer());
}

/// LayoutConstraintLoc - Provides source location information for a
/// parsed layout constraint.
struct LayoutConstraintLoc {
private:
  LayoutConstraint Layout;
  SourceLoc Loc;

public:
  LayoutConstraintLoc(LayoutConstraint Layout, SourceLoc Loc)
      : Layout(Layout), Loc(Loc) {}

  bool isError() const;

  /// Get the representative location of this type, for diagnostic
  /// purposes.
  SourceLoc getLoc() const { return Loc; }

  SourceRange getSourceRange() const;

  bool hasLocation() const { return Loc.isValid(); }
  LayoutConstraint getLayoutConstraint() const { return Layout; }

  bool isNull() const { return Layout.isNull(); }
};

/// Checks if ID is a name of a layout constraint and returns this
/// constraint. If ID does not match any known layout constraint names,
/// returns UnknownLayout.
LayoutConstraint getLayoutConstraint(Identifier ID, ASTContext &Ctx);

} // end namespace language

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::LayoutConstraintInfo, language::TypeAlignInBits)

namespace toolchain {
static inline raw_ostream &
operator<<(raw_ostream &OS, language::LayoutConstraint LC) {
  LC->print(OS);
  return OS;
}

// A LayoutConstraint casts like a LayoutConstraintInfo*.
template <> struct simplify_type<const ::language::LayoutConstraint> {
  typedef ::language::LayoutConstraintInfo *SimpleType;
  static SimpleType getSimplifiedValue(const ::language::LayoutConstraint &Val) {
    return Val.getPointer();
  }
};

template <>
struct simplify_type<::language::LayoutConstraint>
    : public simplify_type<const ::language::LayoutConstraint> {};

// LayoutConstraint hashes just like pointers.
template <> struct DenseMapInfo<language::LayoutConstraint> {
  static language::LayoutConstraint getEmptyKey() {
    return toolchain::DenseMapInfo<language::LayoutConstraintInfo *>::getEmptyKey();
  }
  static language::LayoutConstraint getTombstoneKey() {
    return toolchain::DenseMapInfo<language::LayoutConstraintInfo *>::getTombstoneKey();
  }
  static unsigned getHashValue(language::LayoutConstraint Val) {
    return DenseMapInfo<language::LayoutConstraintInfo *>::getHashValue(
        Val.getPointer());
  }
  static bool isEqual(language::LayoutConstraint LHS,
                      language::LayoutConstraint RHS) {
    return LHS.getPointer() == RHS.getPointer();
  }
};

// A LayoutConstraint is "pointer like".
template <> struct PointerLikeTypeTraits<language::LayoutConstraint> {
public:
  static inline void *getAsVoidPointer(language::LayoutConstraint I) {
    return (void *)I.getPointer();
  }
  static inline language::LayoutConstraint getFromVoidPointer(void *P) {
    return (language::LayoutConstraintInfo *)P;
  }
  enum { NumLowBitsAvailable = language::TypeAlignInBits };
};
} // end namespace toolchain

#endif
