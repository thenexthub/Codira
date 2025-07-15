//===--- CaptureInfo.h - Data Structure for Capture Lists -------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_CAPTURE_INFO_H
#define LANGUAGE_AST_CAPTURE_INFO_H

#include "language/Basic/Debug.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/OptionSet.h"
#include "language/Basic/SourceLoc.h"
#include "language/AST/Type.h"
#include "language/AST/TypeAlignments.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/Support/TrailingObjects.h"

namespace language {
class CapturedValue;
} // namespace language

namespace language {
namespace Lowering {
class TypeConverter;
} // namespace Lowering
} // namespace language

namespace toolchain {
class raw_ostream;
template <> struct DenseMapInfo<language::CapturedValue>;
} // namespace toolchain

namespace language {
class ValueDecl;
class FuncDecl;
class Expr;
class OpaqueValueExpr;
class PackElementExpr;
class VarDecl;
class GenericEnvironment;
class Type;

/// CapturedValue includes both the declaration being captured, along with flags
/// that indicate how it is captured.
class CapturedValue {
  friend class Lowering::TypeConverter;

public:
  using Storage =
      toolchain::PointerIntPair<toolchain::PointerUnion<ValueDecl *, Expr *>, 2,
                           unsigned>;

private:
  Storage Value;
  SourceLoc Loc;

  explicit CapturedValue(Storage V, SourceLoc Loc) : Value(V), Loc(Loc) {}

public:
  friend struct toolchain::DenseMapInfo<CapturedValue>;

  enum {
    /// IsDirect is set when a VarDecl with storage *and* accessors is captured
    /// by its storage address.  This happens in the accessors for the VarDecl.
    IsDirect = 1 << 0,

    /// IsNoEscape is set when a vardecl is captured by a noescape closure, and
    /// thus has its lifetime guaranteed.  It can be closed over by a fixed
    /// address if it has storage.
    IsNoEscape = 1 << 1
  };

  CapturedValue(ValueDecl *Val, unsigned Flags, SourceLoc Loc)
      : Value(Val, Flags), Loc(Loc) {}

  CapturedValue(Expr *Val, unsigned Flags);

public:
  static CapturedValue getDynamicSelfMetadata() {
    return CapturedValue((ValueDecl *)nullptr, 0, SourceLoc());
  }

  bool isDirect() const { return Value.getInt() & IsDirect; }
  bool isNoEscape() const { return Value.getInt() & IsNoEscape; }

  bool isDynamicSelfMetadata() const { return !Value.getPointer(); }

  bool isExpr() const {
    return Value.getPointer().dyn_cast<Expr *>();
  }

  bool isPackElement() const;
  bool isOpaqueValue() const;

  /// Returns true if this captured value is a local capture.
  ///
  /// NOTE: This implies that the value is not dynamic self metadata, since
  /// values with decls are the only values that are able to be local captures.
  bool isLocalCapture() const;

  CapturedValue mergeFlags(unsigned flags) const {
    return CapturedValue(Storage(Value.getPointer(), getFlags() & flags), Loc);
  }

  ValueDecl *getDecl() const {
    return Value.getPointer().dyn_cast<ValueDecl *>();
  }

  Expr *getExpr() const {
    return Value.getPointer().dyn_cast<Expr *>();
  }

  OpaqueValueExpr *getOpaqueValue() const;

  PackElementExpr *getPackElement() const;

  Type getPackElementType() const;

  SourceLoc getLoc() const { return Loc; }

  unsigned getFlags() const { return Value.getInt(); }
};

/// Describes a type that has been captured by a closure or local function.
class CapturedType {
  Type type;
  SourceLoc loc;

public:
  CapturedType(Type type, SourceLoc loc) : type(type), loc(loc) { }
  
  Type getType() const { return type; }
  SourceLoc getLoc() const { return loc; }
};

} // end language namespace

namespace language {

class DynamicSelfType;

/// Stores information about captured variables.
class CaptureInfo {
  class CaptureInfoStorage final
      : public toolchain::TrailingObjects<CaptureInfoStorage,
                                     CapturedValue,
                                     GenericEnvironment *,
                                     CapturedType> {

    DynamicSelfType *DynamicSelf;
    OpaqueValueExpr *OpaqueValue;
    unsigned NumCapturedValues;
    unsigned NumGenericEnvironments;
    unsigned NumCapturedTypes;

  public:
    explicit CaptureInfoStorage(DynamicSelfType *dynamicSelf,
                                OpaqueValueExpr *opaqueValue,
                                unsigned numCapturedValues,
                                unsigned numGenericEnvironments,
                                unsigned numCapturedTypes)
      : DynamicSelf(dynamicSelf), OpaqueValue(opaqueValue),
        NumCapturedValues(numCapturedValues),
        NumGenericEnvironments(numGenericEnvironments),
        NumCapturedTypes(numCapturedTypes) { }

    ArrayRef<CapturedValue> getCaptures() const;

    ArrayRef<GenericEnvironment *> getGenericEnvironments() const;

    ArrayRef<CapturedType> getCapturedTypes() const;

    DynamicSelfType *getDynamicSelfType() const {
      return DynamicSelf;
    }

    OpaqueValueExpr *getOpaqueValue() const {
      return OpaqueValue;
    }

    unsigned numTrailingObjects(OverloadToken<CapturedValue>) const {
      return NumCapturedValues;
    }

    unsigned numTrailingObjects(OverloadToken<GenericEnvironment *>) const {
      return NumGenericEnvironments;
    }

    unsigned numTrailingObjects(OverloadToken<CapturedType>) const {
      return NumCapturedTypes;
    }
  };

  enum class Flags : unsigned {
    HasGenericParamCaptures = 1 << 0
  };

  toolchain::PointerIntPair<const CaptureInfoStorage *, 2, OptionSet<Flags>>
      StorageAndFlags;

public:
  /// The default-constructed CaptureInfo is "not yet computed".
  CaptureInfo() = default;
  CaptureInfo(ASTContext &ctx,
              ArrayRef<CapturedValue> captures,
              DynamicSelfType *dynamicSelf, OpaqueValueExpr *opaqueValue,
              bool genericParamCaptures,
              ArrayRef<GenericEnvironment *> genericEnv=ArrayRef<GenericEnvironment*>(),
              ArrayRef<CapturedType> capturedTypes = ArrayRef<CapturedType>());

  /// A CaptureInfo representing no captures at all.
  static CaptureInfo empty();

  bool hasBeenComputed() const {
    return StorageAndFlags.getPointer();
  }

  bool isTrivial() const;

  /// Returns all captured values and opaque expressions.
  ArrayRef<CapturedValue> getCaptures() const {
    assert(hasBeenComputed());
    return StorageAndFlags.getPointer()->getCaptures();
  }

  /// Returns all captured pack element environments.
  ArrayRef<GenericEnvironment *> getGenericEnvironments() const {
    assert(hasBeenComputed());
    return StorageAndFlags.getPointer()->getGenericEnvironments();
  }

  /// Returns all captured values and opaque expressions.
  ArrayRef<CapturedType> getCapturedTypes() const {
    assert(hasBeenComputed());
    return StorageAndFlags.getPointer()->getCapturedTypes();
  }

  /// \returns true if the function captures the primary generic environment
  /// from its innermost declaration context.
  bool hasGenericParamCaptures() const {
    assert(hasBeenComputed());
    return StorageAndFlags.getInt().contains(Flags::HasGenericParamCaptures);
  }

  /// \returns true if the function captures the dynamic Self type.
  bool hasDynamicSelfCapture() const {
    return getDynamicSelfType() != nullptr;
  }

  /// \returns the captured dynamic Self type, if any.
  DynamicSelfType *getDynamicSelfType() const {
    assert(hasBeenComputed());
    return StorageAndFlags.getPointer()->getDynamicSelfType();
  }

  bool hasOpaqueValueCapture() const {
    assert(hasBeenComputed());
    return getOpaqueValue() != nullptr;
  }

  OpaqueValueExpr *getOpaqueValue() const {
    assert(hasBeenComputed());
    return StorageAndFlags.getPointer()->getOpaqueValue();
  }

  /// Retrieve the variable corresponding to an isolated parameter that has
  /// been captured, if there is one. This might be a capture variable
  /// that was initialized with an isolated parameter.
  VarDecl *getIsolatedParamCapture() const;

  LANGUAGE_DEBUG_DUMP;
  void print(raw_ostream &OS) const;
};

} // namespace language

#endif // TOOLCHAIN_LANGUAGE_AST_CAPTURE_INFO_H

