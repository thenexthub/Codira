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
// These classes implement wrappers around mlir::Value in order to fully
// represent the range of values for C L- and R- values.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_LIB_CIR_CIRGENVALUE_H
#define CLANG_LIB_CIR_CIRGENVALUE_H

#include "Address.h"

#include "language/Core/AST/CharUnits.h"
#include "language/Core/AST/Type.h"

#include "CIRGenRecordLayout.h"
#include "mlir/IR/Value.h"

#include "language/Core/CIR/MissingFeatures.h"

namespace language::Core::CIRGen {

/// This trivial value class is used to represent the result of an
/// expression that is evaluated. It can be one of three things: either a
/// simple MLIR SSA value, a pair of SSA values for complex numbers, or the
/// address of an aggregate value in memory.
class RValue {
  enum Flavor { Scalar, Complex, Aggregate };

  union {
    mlir::Value value;

    // Stores aggregate address.
    Address aggregateAddr;
  };

  unsigned isVolatile : 1;
  unsigned flavor : 2;

public:
  RValue() : value(nullptr), flavor(Scalar) {}

  bool isScalar() const { return flavor == Scalar; }
  bool isComplex() const { return flavor == Complex; }
  bool isAggregate() const { return flavor == Aggregate; }

  bool isVolatileQualified() const { return isVolatile; }

  /// Return the value of this scalar value.
  mlir::Value getValue() const {
    assert(isScalar() && "Not a scalar!");
    return value;
  }

  /// Return the value of this complex value.
  mlir::Value getComplexValue() const {
    assert(isComplex() && "Not a complex!");
    return value;
  }

  /// Return the value of the address of the aggregate.
  Address getAggregateAddress() const {
    assert(isAggregate() && "Not an aggregate!");
    return aggregateAddr;
  }

  mlir::Value getAggregatePointer(QualType pointeeType) const {
    return getAggregateAddress().getPointer();
  }

  static RValue getIgnored() {
    // FIXME: should we make this a more explicit state?
    return get(nullptr);
  }

  static RValue get(mlir::Value v) {
    RValue er;
    er.value = v;
    er.flavor = Scalar;
    er.isVolatile = false;
    return er;
  }

  static RValue getComplex(mlir::Value v) {
    RValue er;
    er.value = v;
    er.flavor = Complex;
    er.isVolatile = false;
    return er;
  }

  // volatile or not.  Remove default to find all places that probably get this
  // wrong.

  /// Convert an Address to an RValue. If the Address is not
  /// signed, create an RValue using the unsigned address. Otherwise, resign the
  /// address using the provided type.
  static RValue getAggregate(Address addr, bool isVolatile = false) {
    RValue er;
    er.aggregateAddr = addr;
    er.flavor = Aggregate;
    er.isVolatile = isVolatile;
    return er;
  }
};

/// The source of the alignment of an l-value; an expression of
/// confidence in the alignment actually matching the estimate.
enum class AlignmentSource {
  /// The l-value was an access to a declared entity or something
  /// equivalently strong, like the address of an array allocated by a
  /// language runtime.
  Decl,

  /// The l-value was considered opaque, so the alignment was
  /// determined from a type, but that type was an explicitly-aligned
  /// typedef.
  AttributedType,

  /// The l-value was considered opaque, so the alignment was
  /// determined from a type.
  Type
};

/// Given that the base address has the given alignment source, what's
/// our confidence in the alignment of the field?
static inline AlignmentSource getFieldAlignmentSource(AlignmentSource source) {
  // For now, we don't distinguish fields of opaque pointers from
  // top-level declarations, but maybe we should.
  return AlignmentSource::Decl;
}

class LValueBaseInfo {
  AlignmentSource alignSource;

public:
  explicit LValueBaseInfo(AlignmentSource source = AlignmentSource::Type)
      : alignSource(source) {}
  AlignmentSource getAlignmentSource() const { return alignSource; }
  void setAlignmentSource(AlignmentSource source) { alignSource = source; }

  void mergeForCast(const LValueBaseInfo &info) {
    setAlignmentSource(info.getAlignmentSource());
  }
};

class LValue {
  enum {
    Simple,       // This is a normal l-value, use getAddress().
    VectorElt,    // This is a vector element l-value (V[i]), use getVector*
    BitField,     // This is a bitfield l-value, use getBitfield*.
    ExtVectorElt, // This is an extended vector subset, use getExtVectorComp
    GlobalReg,    // This is a register l-value, use getGlobalReg()
    MatrixElt     // This is a matrix element, use getVector*
  } lvType;
  language::Core::QualType type;
  language::Core::Qualifiers quals;

  // The alignment to use when accessing this lvalue. (For vector elements,
  // this is the alignment of the whole vector)
  unsigned alignment;
  mlir::Value v;
  mlir::Value vectorIdx; // Index for vector subscript
  mlir::Type elementType;
  LValueBaseInfo baseInfo;
  const CIRGenBitFieldInfo *bitFieldInfo{nullptr};

  void initialize(language::Core::QualType type, language::Core::Qualifiers quals,
                  language::Core::CharUnits alignment, LValueBaseInfo baseInfo) {
    assert((!alignment.isZero() || type->isIncompleteType()) &&
           "initializing l-value with zero alignment!");
    this->type = type;
    this->quals = quals;
    const unsigned maxAlign = 1U << 31;
    this->alignment = alignment.getQuantity() <= maxAlign
                          ? alignment.getQuantity()
                          : maxAlign;
    assert(this->alignment == alignment.getQuantity() &&
           "Alignment exceeds allowed max!");
    this->baseInfo = baseInfo;
  }

public:
  bool isSimple() const { return lvType == Simple; }
  bool isVectorElt() const { return lvType == VectorElt; }
  bool isBitField() const { return lvType == BitField; }
  bool isGlobalReg() const { return lvType == GlobalReg; }
  bool isVolatile() const { return quals.hasVolatile(); }

  bool isVolatileQualified() const { return quals.hasVolatile(); }

  unsigned getVRQualifiers() const {
    return quals.getCVRQualifiers() & ~language::Core::Qualifiers::Const;
  }

  language::Core::QualType getType() const { return type; }

  mlir::Value getPointer() const { return v; }

  language::Core::CharUnits getAlignment() const {
    return language::Core::CharUnits::fromQuantity(alignment);
  }
  void setAlignment(language::Core::CharUnits a) { alignment = a.getQuantity(); }

  Address getAddress() const {
    return Address(getPointer(), elementType, getAlignment());
  }

  const language::Core::Qualifiers &getQuals() const { return quals; }
  language::Core::Qualifiers &getQuals() { return quals; }

  LValueBaseInfo getBaseInfo() const { return baseInfo; }
  void setBaseInfo(LValueBaseInfo info) { baseInfo = info; }

  static LValue makeAddr(Address address, language::Core::QualType t,
                         LValueBaseInfo baseInfo) {
    // Classic codegen sets the objc gc qualifier here. That requires an
    // ASTContext, which is passed in from CIRGenFunction::makeAddrLValue.
    assert(!cir::MissingFeatures::objCGC());

    LValue r;
    r.lvType = Simple;
    r.v = address.getPointer();
    r.elementType = address.getElementType();
    r.initialize(t, t.getQualifiers(), address.getAlignment(), baseInfo);
    return r;
  }

  Address getVectorAddress() const {
    return Address(getVectorPointer(), elementType, getAlignment());
  }

  mlir::Value getVectorPointer() const {
    assert(isVectorElt());
    return v;
  }

  mlir::Value getVectorIdx() const {
    assert(isVectorElt());
    return vectorIdx;
  }

  static LValue makeVectorElt(Address vecAddress, mlir::Value index,
                              language::Core::QualType t, LValueBaseInfo baseInfo) {
    LValue r;
    r.lvType = VectorElt;
    r.v = vecAddress.getPointer();
    r.elementType = vecAddress.getElementType();
    r.vectorIdx = index;
    r.initialize(t, t.getQualifiers(), vecAddress.getAlignment(), baseInfo);
    return r;
  }

  // bitfield lvalue
  Address getBitFieldAddress() const {
    return Address(getBitFieldPointer(), elementType, getAlignment());
  }

  mlir::Value getBitFieldPointer() const {
    assert(isBitField());
    return v;
  }

  const CIRGenBitFieldInfo &getBitFieldInfo() const {
    assert(isBitField());
    return *bitFieldInfo;
  }

  /// Create a new object to represent a bit-field access.
  ///
  /// \param Addr - The base address of the bit-field sequence this
  /// bit-field refers to.
  /// \param Info - The information describing how to perform the bit-field
  /// access.
  static LValue makeBitfield(Address addr, const CIRGenBitFieldInfo &info,
                             language::Core::QualType type, LValueBaseInfo baseInfo) {
    LValue r;
    r.lvType = BitField;
    r.v = addr.getPointer();
    r.elementType = addr.getElementType();
    r.bitFieldInfo = &info;
    r.initialize(type, type.getQualifiers(), addr.getAlignment(), baseInfo);
    return r;
  }
};

/// An aggregate value slot.
class AggValueSlot {

  Address addr;
  language::Core::Qualifiers quals;

  /// This is set to true if some external code is responsible for setting up a
  /// destructor for the slot.  Otherwise the code which constructs it should
  /// push the appropriate cleanup.
  LLVM_PREFERRED_TYPE(bool)
  LLVM_ATTRIBUTE_UNUSED unsigned destructedFlag : 1;

  /// This is set to true if the memory in the slot is known to be zero before
  /// the assignment into it.  This means that zero fields don't need to be set.
  LLVM_PREFERRED_TYPE(bool)
  unsigned zeroedFlag : 1;

  /// This is set to true if the slot might be aliased and it's not undefined
  /// behavior to access it through such an alias.  Note that it's always
  /// undefined behavior to access a C++ object that's under construction
  /// through an alias derived from outside the construction process.
  ///
  /// This flag controls whether calls that produce the aggregate
  /// value may be evaluated directly into the slot, or whether they
  /// must be evaluated into an unaliased temporary and then memcpy'ed
  /// over.  Since it's invalid in general to memcpy a non-POD C++
  /// object, it's important that this flag never be set when
  /// evaluating an expression which constructs such an object.
  LLVM_PREFERRED_TYPE(bool)
  LLVM_ATTRIBUTE_UNUSED unsigned aliasedFlag : 1;

  /// This is set to true if the tail padding of this slot might overlap
  /// another object that may have already been initialized (and whose
  /// value must be preserved by this initialization). If so, we may only
  /// store up to the dsize of the type. Otherwise we can widen stores to
  /// the size of the type.
  LLVM_PREFERRED_TYPE(bool)
  LLVM_ATTRIBUTE_UNUSED unsigned overlapFlag : 1;

public:
  enum IsDestructed_t { IsNotDestructed, IsDestructed };
  enum IsZeroed_t { IsNotZeroed, IsZeroed };
  enum IsAliased_t { IsNotAliased, IsAliased };
  enum Overlap_t { MayOverlap, DoesNotOverlap };

  /// Returns an aggregate value slot indicating that the aggregate
  /// value is being ignored.
  static AggValueSlot ignored() {
    return forAddr(Address::invalid(), language::Core::Qualifiers(), IsNotDestructed,
                   IsNotAliased, DoesNotOverlap);
  }

  AggValueSlot(Address addr, language::Core::Qualifiers quals, bool destructedFlag,
               bool zeroedFlag, bool aliasedFlag, bool overlapFlag)
      : addr(addr), quals(quals), destructedFlag(destructedFlag),
        zeroedFlag(zeroedFlag), aliasedFlag(aliasedFlag),
        overlapFlag(overlapFlag) {}

  static AggValueSlot forAddr(Address addr, language::Core::Qualifiers quals,
                              IsDestructed_t isDestructed,
                              IsAliased_t isAliased, Overlap_t mayOverlap,
                              IsZeroed_t isZeroed = IsNotZeroed) {
    return AggValueSlot(addr, quals, isDestructed, isZeroed, isAliased,
                        mayOverlap);
  }

  static AggValueSlot forLValue(const LValue &LV, IsDestructed_t isDestructed,
                                IsAliased_t isAliased, Overlap_t mayOverlap,
                                IsZeroed_t isZeroed = IsNotZeroed) {
    return forAddr(LV.getAddress(), LV.getQuals(), isDestructed, isAliased,
                   mayOverlap, isZeroed);
  }

  language::Core::Qualifiers getQualifiers() const { return quals; }

  Address getAddress() const { return addr; }

  bool isIgnored() const { return !addr.isValid(); }

  mlir::Value getPointer() const { return addr.getPointer(); }

  IsZeroed_t isZeroed() const { return IsZeroed_t(zeroedFlag); }

  RValue asRValue() const {
    if (isIgnored())
      return RValue::getIgnored();
    assert(!cir::MissingFeatures::aggValueSlot());
    return RValue::getAggregate(getAddress());
  }
};

} // namespace language::Core::CIRGen

#endif // CLANG_LIB_CIR_CIRGENVALUE_H
