//===- CanonicalType.h - C Language Family Type Representation --*- C++ -*-===//
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
//  This file defines the CanQual class template, which provides access to
//  canonical types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_CANONICALTYPE_H
#define LANGUAGE_CORE_AST_CANONICALTYPE_H

#include "language/Core/AST/Type.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/iterator.h"
#include "toolchain/Support/Casting.h"
#include "toolchain/Support/PointerLikeTypeTraits.h"
#include <cassert>
#include <iterator>
#include <type_traits>

namespace language::Core {

template<typename T> class CanProxy;
template<typename T> struct CanProxyAdaptor;
class ASTContext;
class CXXRecordDecl;
class EnumDecl;
class Expr;
class IdentifierInfo;
class ObjCInterfaceDecl;
class RecordDecl;
class TagDecl;
class TemplateTypeParmDecl;

//----------------------------------------------------------------------------//
// Canonical, qualified type template
//----------------------------------------------------------------------------//

/// Represents a canonical, potentially-qualified type.
///
/// The CanQual template is a lightweight smart pointer that provides access
/// to the canonical representation of a type, where all typedefs and other
/// syntactic sugar has been eliminated. A CanQualType may also have various
/// qualifiers (const, volatile, restrict) attached to it.
///
/// The template type parameter @p T is one of the Type classes (PointerType,
/// BuiltinType, etc.). The type stored within @c CanQual<T> will be of that
/// type (or some subclass of that type). The typedef @c CanQualType is just
/// a shorthand for @c CanQual<Type>.
///
/// An instance of @c CanQual<T> can be implicitly converted to a
/// @c CanQual<U> when T is derived from U, which essentially provides an
/// implicit upcast. For example, @c CanQual<LValueReferenceType> can be
/// converted to @c CanQual<ReferenceType>. Note that any @c CanQual type can
/// be implicitly converted to a QualType, but the reverse operation requires
/// a call to ASTContext::getCanonicalType().
template<typename T = Type>
class CanQual {
  /// The actual, canonical type.
  QualType Stored;

public:
  /// Constructs a NULL canonical type.
  CanQual() = default;

  /// Converting constructor that permits implicit upcasting of
  /// canonical type pointers.
  template <typename U>
  CanQual(const CanQual<U> &Other,
          std::enable_if_t<std::is_base_of<T, U>::value, int> = 0);

  /// Retrieve the underlying type pointer, which refers to a
  /// canonical type.
  ///
  /// The underlying pointer must not be nullptr.
  const T *getTypePtr() const { return cast<T>(Stored.getTypePtr()); }

  /// Retrieve the underlying type pointer, which refers to a
  /// canonical type, or nullptr.
  const T *getTypePtrOrNull() const {
    return cast_or_null<T>(Stored.getTypePtrOrNull());
  }

  /// Implicit conversion to a qualified type.
  operator QualType() const { return Stored; }

  /// Implicit conversion to bool.
  explicit operator bool() const { return !isNull(); }

  bool isNull() const {
    return Stored.isNull();
  }

  SplitQualType split() const { return Stored.split(); }

  /// Retrieve a canonical type pointer with a different static type,
  /// upcasting or downcasting as needed.
  ///
  /// The getAs() function is typically used to try to downcast to a
  /// more specific (canonical) type in the type system. For example:
  ///
  /// @code
  /// void f(CanQual<Type> T) {
  ///   if (CanQual<PointerType> Ptr = T->getAs<PointerType>()) {
  ///     // look at Ptr's pointee type
  ///   }
  /// }
  /// @endcode
  ///
  /// \returns A proxy pointer to the same type, but with the specified
  /// static type (@p U). If the dynamic type is not the specified static type
  /// or a derived class thereof, a NULL canonical type.
  template<typename U> CanProxy<U> getAs() const;

  template<typename U> CanProxy<U> castAs() const;

  /// Overloaded arrow operator that produces a canonical type
  /// proxy.
  CanProxy<T> operator->() const;

  /// Retrieve all qualifiers.
  Qualifiers getQualifiers() const { return Stored.getLocalQualifiers(); }

  /// Retrieve the const/volatile/restrict qualifiers.
  unsigned getCVRQualifiers() const { return Stored.getLocalCVRQualifiers(); }

  /// Determines whether this type has any qualifiers
  bool hasQualifiers() const { return Stored.hasLocalQualifiers(); }

  bool isConstQualified() const {
    return Stored.isLocalConstQualified();
  }

  bool isVolatileQualified() const {
    return Stored.isLocalVolatileQualified();
  }

  bool isRestrictQualified() const {
    return Stored.isLocalRestrictQualified();
  }

  /// Determines if this canonical type is furthermore
  /// canonical as a parameter.  The parameter-canonicalization
  /// process decays arrays to pointers and drops top-level qualifiers.
  bool isCanonicalAsParam() const {
    return Stored.isCanonicalAsParam();
  }

  /// Retrieve the unqualified form of this type.
  CanQual<T> getUnqualifiedType() const;

  /// Retrieves a version of this type with const applied.
  /// Note that this does not always yield a canonical type.
  QualType withConst() const {
    return Stored.withConst();
  }

  /// Determines whether this canonical type is more qualified than
  /// the @p Other canonical type.
  bool isMoreQualifiedThan(CanQual<T> Other, const ASTContext &Ctx) const {
    return Stored.isMoreQualifiedThan(Other.Stored, Ctx);
  }

  /// Determines whether this canonical type is at least as qualified as
  /// the @p Other canonical type.
  bool isAtLeastAsQualifiedAs(CanQual<T> Other, const ASTContext &Ctx) const {
    return Stored.isAtLeastAsQualifiedAs(Other.Stored, Ctx);
  }

  /// If the canonical type is a reference type, returns the type that
  /// it refers to; otherwise, returns the type itself.
  CanQual<Type> getNonReferenceType() const;

  /// Retrieve the internal representation of this canonical type.
  void *getAsOpaquePtr() const { return Stored.getAsOpaquePtr(); }

  /// Construct a canonical type from its internal representation.
  static CanQual<T> getFromOpaquePtr(void *Ptr);

  /// Builds a canonical type from a QualType.
  ///
  /// This routine is inherently unsafe, because it requires the user to
  /// ensure that the given type is a canonical type with the correct
  // (dynamic) type.
  static CanQual<T> CreateUnsafe(QualType Other);

  void dump() const { Stored.dump(); }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    ID.AddPointer(getAsOpaquePtr());
  }
};

template<typename T, typename U>
inline bool operator==(CanQual<T> x, CanQual<U> y) {
  return x.getAsOpaquePtr() == y.getAsOpaquePtr();
}

template<typename T, typename U>
inline bool operator!=(CanQual<T> x, CanQual<U> y) {
  return x.getAsOpaquePtr() != y.getAsOpaquePtr();
}

/// Represents a canonical, potentially-qualified type.
using CanQualType = CanQual<Type>;

inline CanQualType Type::getCanonicalTypeUnqualified() const {
  return CanQualType::CreateUnsafe(getCanonicalTypeInternal());
}

inline const StreamingDiagnostic &operator<<(const StreamingDiagnostic &DB,
                                             CanQualType T) {
  DB << static_cast<QualType>(T);
  return DB;
}

//----------------------------------------------------------------------------//
// Internal proxy classes used by canonical types
//----------------------------------------------------------------------------//

#define LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(Accessor)                    \
CanQualType Accessor() const {                                           \
return CanQualType::CreateUnsafe(this->getTypePtr()->Accessor());      \
}

#define LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(Type, Accessor)             \
Type Accessor() const { return this->getTypePtr()->Accessor(); }

/// Base class of all canonical proxy types, which is responsible for
/// storing the underlying canonical type and providing basic conversions.
template<typename T>
class CanProxyBase {
protected:
  CanQual<T> Stored;

public:
  /// Retrieve the pointer to the underlying Type
  const T *getTypePtr() const { return Stored.getTypePtr(); }

  /// Implicit conversion to the underlying pointer.
  ///
  /// Also provides the ability to use canonical type proxies in a Boolean
  // context,e.g.,
  /// @code
  ///   if (CanQual<PointerType> Ptr = T->getAs<PointerType>()) { ... }
  /// @endcode
  operator const T*() const { return this->Stored.getTypePtrOrNull(); }

  /// Try to convert the given canonical type to a specific structural
  /// type.
  template<typename U> CanProxy<U> getAs() const {
    return this->Stored.template getAs<U>();
  }

  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(Type::TypeClass, getTypeClass)

  // Type predicates
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjectType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isIncompleteType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isSizelessType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isSizelessBuiltinType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isIncompleteOrObjectType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isVariablyModifiedType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isEnumeralType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isBooleanType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isCharType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isWideCharType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegralType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegralOrEnumerationType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isRealFloatingType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isComplexType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isAnyComplexType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isFloatingType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isRealType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isArithmeticType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isVoidType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isDerivedType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isScalarType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isAggregateType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isAnyPointerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isVoidPointerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isFunctionPointerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isMemberFunctionPointerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isClassType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isStructureType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isInterfaceType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isStructureOrClassType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isUnionType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isComplexIntegerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isNullPtrType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isDependentType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isOverloadableType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isArrayType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isConstantArrayType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasPointerRepresentation)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasObjCPointerRepresentation)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasIntegerRepresentation)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasSignedIntegerRepresentation)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasUnsignedIntegerRepresentation)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasFloatingRepresentation)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isSignedIntegerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isUnsignedIntegerType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isSignedIntegerOrEnumerationType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isUnsignedIntegerOrEnumerationType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isConstantSizeType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isSpecifierType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(CXXRecordDecl*, getAsCXXRecordDecl)

  /// Retrieve the proxy-adaptor type.
  ///
  /// This arrow operator is used when CanProxyAdaptor has been specialized
  /// for the given type T. In that case, we reference members of the
  /// CanProxyAdaptor specialization. Otherwise, this operator will be hidden
  /// by the arrow operator in the primary CanProxyAdaptor template.
  const CanProxyAdaptor<T> *operator->() const {
    return static_cast<const CanProxyAdaptor<T> *>(this);
  }
};

/// Replaceable canonical proxy adaptor class that provides the link
/// between a canonical type and the accessors of the type.
///
/// The CanProxyAdaptor is a replaceable class template that is instantiated
/// as part of each canonical proxy type. The primary template merely provides
/// redirection to the underlying type (T), e.g., @c PointerType. One can
/// provide specializations of this class template for each underlying type
/// that provide accessors returning canonical types (@c CanQualType) rather
/// than the more typical @c QualType, to propagate the notion of "canonical"
/// through the system.
template<typename T>
struct CanProxyAdaptor : CanProxyBase<T> {};

/// Canonical proxy type returned when retrieving the members of a
/// canonical type or as the result of the @c CanQual<T>::getAs member
/// function.
///
/// The CanProxy type mainly exists as a proxy through which operator-> will
/// look to either map down to a raw T* (e.g., PointerType*) or to a proxy
/// type that provides canonical-type access to the fields of the type.
template<typename T>
class CanProxy : public CanProxyAdaptor<T> {
public:
  /// Build a NULL proxy.
  CanProxy() = default;

  /// Build a proxy to the given canonical type.
  CanProxy(CanQual<T> Stored) { this->Stored = Stored; }

  /// Implicit conversion to the stored canonical type.
  operator CanQual<T>() const { return this->Stored; }
};

} // namespace language::Core

namespace toolchain {

/// Implement simplify_type for CanQual<T>, so that we can dyn_cast from
/// CanQual<T> to a specific Type class. We're prefer isa/dyn_cast/cast/etc.
/// to return smart pointer (proxies?).
template<typename T>
struct simplify_type< ::language::Core::CanQual<T>> {
  using SimpleType = const T *;

  static SimpleType getSimplifiedValue(::language::Core::CanQual<T> Val) {
    return Val.getTypePtr();
  }
};

// Teach SmallPtrSet that CanQual<T> is "basically a pointer".
template<typename T>
struct PointerLikeTypeTraits<language::Core::CanQual<T>> {
  static void *getAsVoidPointer(language::Core::CanQual<T> P) {
    return P.getAsOpaquePtr();
  }

  static language::Core::CanQual<T> getFromVoidPointer(void *P) {
    return language::Core::CanQual<T>::getFromOpaquePtr(P);
  }

  // qualifier information is encoded in the low bits.
  static constexpr int NumLowBitsAvailable = 0;
};

} // namespace toolchain

namespace language::Core {

//----------------------------------------------------------------------------//
// Canonical proxy adaptors for canonical type nodes.
//----------------------------------------------------------------------------//

/// Iterator adaptor that turns an iterator over canonical QualTypes
/// into an iterator over CanQualTypes.
template <typename InputIterator>
struct CanTypeIterator
    : toolchain::iterator_adaptor_base<
          CanTypeIterator<InputIterator>, InputIterator,
          typename std::iterator_traits<InputIterator>::iterator_category,
          CanQualType,
          typename std::iterator_traits<InputIterator>::difference_type,
          CanProxy<Type>, CanQualType> {
  CanTypeIterator() = default;
  explicit CanTypeIterator(InputIterator Iter)
      : CanTypeIterator::iterator_adaptor_base(std::move(Iter)) {}

  CanQualType operator*() const { return CanQualType::CreateUnsafe(*this->I); }
  CanProxy<Type> operator->() const;
};

template<>
struct CanProxyAdaptor<ComplexType> : public CanProxyBase<ComplexType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getElementType)
};

template<>
struct CanProxyAdaptor<PointerType> : public CanProxyBase<PointerType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<BlockPointerType>
  : public CanProxyBase<BlockPointerType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<ReferenceType> : public CanProxyBase<ReferenceType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<LValueReferenceType>
  : public CanProxyBase<LValueReferenceType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<RValueReferenceType>
  : public CanProxyBase<RValueReferenceType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
};

template<>
struct CanProxyAdaptor<MemberPointerType>
  : public CanProxyBase<MemberPointerType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(NestedNameSpecifier, getQualifier)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(const CXXRecordDecl *,
                                      getMostRecentCXXRecordDecl)
};

// CanProxyAdaptors for arrays are intentionally unimplemented because
// they are not safe.
template<> struct CanProxyAdaptor<ArrayType>;
template<> struct CanProxyAdaptor<ConstantArrayType>;
template<> struct CanProxyAdaptor<IncompleteArrayType>;
template<> struct CanProxyAdaptor<VariableArrayType>;
template<> struct CanProxyAdaptor<DependentSizedArrayType>;

template<>
struct CanProxyAdaptor<DependentSizedExtVectorType>
  : public CanProxyBase<DependentSizedExtVectorType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getElementType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(const Expr *, getSizeExpr)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(SourceLocation, getAttributeLoc)
};

template<>
struct CanProxyAdaptor<VectorType> : public CanProxyBase<VectorType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getElementType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumElements)
};

template<>
struct CanProxyAdaptor<ExtVectorType> : public CanProxyBase<ExtVectorType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getElementType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumElements)
};

template<>
struct CanProxyAdaptor<FunctionType> : public CanProxyBase<FunctionType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getReturnType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(FunctionType::ExtInfo, getExtInfo)
};

template<>
struct CanProxyAdaptor<FunctionNoProtoType>
  : public CanProxyBase<FunctionNoProtoType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getReturnType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(FunctionType::ExtInfo, getExtInfo)
};

template<>
struct CanProxyAdaptor<FunctionProtoType>
  : public CanProxyBase<FunctionProtoType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getReturnType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(FunctionType::ExtInfo, getExtInfo)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumParams)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasExtParameterInfos)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(
            ArrayRef<FunctionProtoType::ExtParameterInfo>, getExtParameterInfos)

  CanQualType getParamType(unsigned i) const {
    return CanQualType::CreateUnsafe(this->getTypePtr()->getParamType(i));
  }

  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isVariadic)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(Qualifiers, getMethodQuals)

  using param_type_iterator =
      CanTypeIterator<FunctionProtoType::param_type_iterator>;

  param_type_iterator param_type_begin() const {
    return param_type_iterator(this->getTypePtr()->param_type_begin());
  }

  param_type_iterator param_type_end() const {
    return param_type_iterator(this->getTypePtr()->param_type_end());
  }

  // Note: canonical function types never have exception specifications
};

template<>
struct CanProxyAdaptor<TypeOfType> : public CanProxyBase<TypeOfType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getUnmodifiedType)
};

template<>
struct CanProxyAdaptor<DecltypeType> : public CanProxyBase<DecltypeType> {
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(Expr *, getUnderlyingExpr)
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getUnderlyingType)
};

template <>
struct CanProxyAdaptor<UnaryTransformType>
    : public CanProxyBase<UnaryTransformType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getBaseType)
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getUnderlyingType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(UnaryTransformType::UTTKind, getUTTKind)
};

template<>
struct CanProxyAdaptor<TagType> : public CanProxyBase<TagType> {
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(TagDecl *, getOriginalDecl)
};

template<>
struct CanProxyAdaptor<RecordType> : public CanProxyBase<RecordType> {
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(RecordDecl *, getOriginalDecl)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, hasConstFields)
};

template<>
struct CanProxyAdaptor<EnumType> : public CanProxyBase<EnumType> {
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(EnumDecl *, getOriginalDecl)
};

template<>
struct CanProxyAdaptor<TemplateTypeParmType>
  : public CanProxyBase<TemplateTypeParmType> {
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getDepth)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getIndex)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isParameterPack)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(TemplateTypeParmDecl *, getDecl)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(IdentifierInfo *, getIdentifier)
};

template<>
struct CanProxyAdaptor<ObjCObjectType>
  : public CanProxyBase<ObjCObjectType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getBaseType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(const ObjCInterfaceDecl *,
                                      getInterface)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCUnqualifiedId)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCUnqualifiedClass)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedId)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedClass)

  using qual_iterator = ObjCObjectPointerType::qual_iterator;

  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_begin)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_end)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, qual_empty)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumProtocols)
};

template<>
struct CanProxyAdaptor<ObjCObjectPointerType>
  : public CanProxyBase<ObjCObjectPointerType> {
  LANGUAGE_CORE_CANPROXY_TYPE_ACCESSOR(getPointeeType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(const ObjCInterfaceType *,
                                      getInterfaceType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCIdType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCClassType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedIdType)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, isObjCQualifiedClassType)

  using qual_iterator = ObjCObjectPointerType::qual_iterator;

  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_begin)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(qual_iterator, qual_end)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(bool, qual_empty)
  LANGUAGE_CORE_CANPROXY_SIMPLE_ACCESSOR(unsigned, getNumProtocols)
};

//----------------------------------------------------------------------------//
// Method and function definitions
//----------------------------------------------------------------------------//
template<typename T>
inline CanQual<T> CanQual<T>::getUnqualifiedType() const {
  return CanQual<T>::CreateUnsafe(Stored.getLocalUnqualifiedType());
}

template<typename T>
inline CanQual<Type> CanQual<T>::getNonReferenceType() const {
  if (CanQual<ReferenceType> RefType = getAs<ReferenceType>())
    return RefType->getPointeeType();
  else
    return *this;
}

template<typename T>
CanQual<T> CanQual<T>::getFromOpaquePtr(void *Ptr) {
  CanQual<T> Result;
  Result.Stored = QualType::getFromOpaquePtr(Ptr);
  assert((!Result || Result.Stored.getAsOpaquePtr() == (void*)-1 ||
          Result.Stored.isCanonical()) && "Type is not canonical!");
  return Result;
}

template<typename T>
CanQual<T> CanQual<T>::CreateUnsafe(QualType Other) {
  assert((Other.isNull() || Other.isCanonical()) && "Type is not canonical!");
  assert((Other.isNull() || isa<T>(Other.getTypePtr())) &&
         "Dynamic type does not meet the static type's requires");
  CanQual<T> Result;
  Result.Stored = Other;
  return Result;
}

template<typename T>
template<typename U>
CanProxy<U> CanQual<T>::getAs() const {
  static_assert(!TypeIsArrayType<T>::value,
                "ArrayType cannot be used with getAs!");

  if (Stored.isNull())
    return CanProxy<U>();

  if (isa<U>(Stored.getTypePtr()))
    return CanQual<U>::CreateUnsafe(Stored);

  return CanProxy<U>();
}

template<typename T>
template<typename U>
CanProxy<U> CanQual<T>::castAs() const {
  static_assert(!TypeIsArrayType<U>::value,
                "ArrayType cannot be used with castAs!");

  assert(!Stored.isNull() && isa<U>(Stored.getTypePtr()));
  return CanQual<U>::CreateUnsafe(Stored);
}

template<typename T>
CanProxy<T> CanQual<T>::operator->() const {
  return CanProxy<T>(*this);
}

template <typename InputIterator>
CanProxy<Type> CanTypeIterator<InputIterator>::operator->() const {
  return CanProxy<Type>(*this);
}

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_CANONICALTYPE_H
