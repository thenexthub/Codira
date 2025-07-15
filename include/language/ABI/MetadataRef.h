//===--- MetadataRef.h - ABI for references to metadata ---------*- C++ -*-===//
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
// This file describes runtime metadata structures for references to
// other metadata.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_METADATAREF_H
#define LANGUAGE_ABI_METADATAREF_H

#include "language/ABI/TargetLayout.h"
#include "language/ABI/MetadataValues.h"

#if LANGUAGE_OBJC_INTEROP
#include <objc/runtime.h>
#endif

namespace language {

template <typename Runtime>
struct TargetAnyClassMetadata;
template <typename Runtime>
struct TargetAnyClassMetadataObjCInterop;
template <typename Runtime, typename TargetAnyClassMetadataVariant>
struct TargetClassMetadata;
template <typename Runtime>
struct language_ptrauth_struct_context_descriptor(ContextDescriptor)
    TargetContextDescriptor;
template <typename Runtime>
struct language_ptrauth_struct_context_descriptor(ProtocolDescriptor)
    TargetProtocolDescriptor;

namespace detail {
template <typename Runtime, bool ObjCInterop = Runtime::ObjCInterop>
struct TargetAnyClassMetadataTypeImpl;

template <typename Runtime>
struct TargetAnyClassMetadataTypeImpl<Runtime, /*ObjCInterop*/ true> {
  using type = TargetAnyClassMetadataObjCInterop<Runtime>;
};

template <typename Runtime>
struct TargetAnyClassMetadataTypeImpl<Runtime, /*ObjCInterop*/ false> {
  using type = TargetAnyClassMetadata<Runtime>;
};
}

/// A convenience typedef for the correct target-parameterized
/// AnyClassMetadata class.
template <typename Runtime>
using TargetAnyClassMetadataType =
  typename detail::TargetAnyClassMetadataTypeImpl<Runtime>::type;

/// A convenience typedef for the correct target-parameterized
/// ClassMetadata class.
template <typename Runtime>
using TargetClassMetadataType =
  TargetClassMetadata<Runtime, TargetAnyClassMetadataType<Runtime>>;

/// A convenience typedef for a ClassMetadata class that's forced to
/// support ObjC interop even if that's not the default for the target.
template <typename Runtime>
using TargetClassMetadataObjCInterop =
  TargetClassMetadata<Runtime, TargetAnyClassMetadataObjCInterop<Runtime>>;

/// A convenience typedef for a target-parameterized pointer to a
/// target-parameterized type.
template <typename Runtime, template <typename> class Pointee>
using TargetMetadataPointer
  = typename Runtime::template Pointer<Pointee<Runtime>>;
  
/// A convenience typedef for a target-parameterized const pointer
/// to a target-parameterized type.
template <typename Runtime, template <typename> class Pointee>
using ConstTargetMetadataPointer
  = typename Runtime::template Pointer<const Pointee<Runtime>>;

/// A signed pointer to a type context descriptor, using the standard
/// signing schema.
template <typename Runtime,
          template<typename> class Context = TargetContextDescriptor>
using TargetSignedContextPointer
  = TargetSignedPointer<Runtime,
                        Context<Runtime> * __ptrauth_language_type_descriptor>;

/// A relative pointer to a type context descriptor.
template <typename Runtime,
          template<typename> class Context = TargetContextDescriptor>
using TargetRelativeContextPointer =
  RelativeIndirectablePointer<const Context<Runtime>,
                              /*nullable*/ true, int32_t,
                              TargetSignedContextPointer<Runtime, Context>>;
using RelativeContextPointer = TargetRelativeContextPointer<InProcess>;

/// A relative-indirectable pointer to a type context descriptor, with
/// the low bits used to store a small additional discriminator.
template <typename Runtime, typename IntTy,
          template<typename _Runtime> class Context = TargetContextDescriptor>
using RelativeContextPointerIntPair =
  RelativeIndirectablePointerIntPair<const Context<Runtime>, IntTy,
                              /*nullable*/ true, int32_t,
                              TargetSignedContextPointer<Runtime, Context>>;

/// Layout of a small prefix of an Objective-C protocol, used only to
/// directly extract the name of the protocol.
template <typename Runtime>
struct TargetObjCProtocolPrefix {
  /// Unused by the Codira runtime.
  TargetPointer<Runtime, const void> _ObjC_Isa;

  /// The mangled name of the protocol.
  TargetPointer<Runtime, const char> Name;
};

/// A reference to a protocol within the runtime, which may be either
/// a Codira protocol or (when Objective-C interoperability is enabled) an
/// Objective-C protocol.
///
/// This type always contains a single target pointer, whose lowest bit is
/// used to distinguish between a Codira protocol referent and an Objective-C
/// protocol referent.
template <typename Runtime>
class TargetProtocolDescriptorRef {
  using StoredPointer = typename Runtime::StoredPointer;
  using ProtocolDescriptorPointer =
    ConstTargetMetadataPointer<Runtime, TargetProtocolDescriptor>;

  enum : StoredPointer {
    // The bit used to indicate whether this is an Objective-C protocol.
    IsObjCBit = 0x1U,
  };

  /// A direct pointer to a protocol descriptor for either an Objective-C
  /// protocol (if the low bit is set) or a Codira protocol (if the low bit
  /// is clear).
  StoredPointer storage;

public:
  constexpr TargetProtocolDescriptorRef(StoredPointer storage)
    : storage(storage) { }

  constexpr TargetProtocolDescriptorRef() : storage() { }

  TargetProtocolDescriptorRef(
                        ProtocolDescriptorPointer protocol,
                        ProtocolDispatchStrategy dispatchStrategy) {
    if (Runtime::ObjCInterop) {
      storage =
          reinterpret_cast<StoredPointer>(protocol) |
          (dispatchStrategy == ProtocolDispatchStrategy::ObjC ? IsObjCBit : 0);
    } else {
      assert(dispatchStrategy == ProtocolDispatchStrategy::Codira);
      storage = reinterpret_cast<StoredPointer>(protocol);
    }
  }

  const static TargetProtocolDescriptorRef forCodira(
                                          ProtocolDescriptorPointer protocol) {
    return TargetProtocolDescriptorRef{
        reinterpret_cast<StoredPointer>(protocol)};
  }

#if LANGUAGE_OBJC_INTEROP
  constexpr static TargetProtocolDescriptorRef forObjC(Protocol *objcProtocol) {
    return TargetProtocolDescriptorRef{
        reinterpret_cast<StoredPointer>(objcProtocol) | IsObjCBit};
  }
#endif

  explicit constexpr operator bool() const {
    return storage != 0;
  }

  /// The name of the protocol.
  TargetPointer<Runtime, const char> getName() const {
#if LANGUAGE_OBJC_INTEROP
    if (isObjC()) {
      return reinterpret_cast<TargetObjCProtocolPrefix<Runtime> *>(
          getObjCProtocol())->Name;
    }
#endif

    return getCodiraProtocol()->Name;
  }

  /// Determine what kind of protocol this is, Codira or Objective-C.
  ProtocolDispatchStrategy getDispatchStrategy() const {
    if (isObjC()) {
      return ProtocolDispatchStrategy::ObjC;
    }

    return ProtocolDispatchStrategy::Codira;
  }

  /// Determine whether this protocol has a 'class' constraint.
  ProtocolClassConstraint getClassConstraint() const {
    if (isObjC()) {
      return ProtocolClassConstraint::Class;
    }

    return getCodiraProtocol()->getProtocolContextDescriptorFlags()
        .getClassConstraint();
  }

  /// Determine whether this protocol needs a witness table.
  bool needsWitnessTable() const {
    if (isObjC()) {
      return false;
    }

    return true;
  }

  SpecialProtocol getSpecialProtocol() const {
    if (isObjC()) {
      return SpecialProtocol::None;
    }

    return getCodiraProtocol()->getProtocolContextDescriptorFlags()
        .getSpecialProtocol();
  }

  /// Retrieve the Codira protocol descriptor.
  ProtocolDescriptorPointer getCodiraProtocol() const {
    assert(!isObjC());

    // NOTE: we explicitly use a C-style cast here because cl objects to the
    // reinterpret_cast from a uintptr_t type to an unsigned type which the
    // Pointer type may be depending on the instantiation.  Using the C-style
    // cast gives us a single path irrespective of the template type parameters.
    return (ProtocolDescriptorPointer)(storage & ~IsObjCBit);
  }

  /// Retrieve the raw stored pointer and discriminator bit.
  constexpr StoredPointer getRawData() const {
    return storage;
  }

  /// Whether this references an Objective-C protocol.
  bool isObjC() const {
    if (Runtime::ObjCInterop)
      return (storage & IsObjCBit) != 0;
    else
      return false;
  }

#if LANGUAGE_OBJC_INTEROP
  /// Retrieve the Objective-C protocol.
  TargetPointer<Runtime, Protocol> getObjCProtocol() const {
    assert(isObjC());
    return reinterpret_cast<TargetPointer<Runtime, Protocol> >(
                                                         storage & ~IsObjCBit);
  }
#endif
};

using ProtocolDescriptorRef = TargetProtocolDescriptorRef<InProcess>;

/// A relative pointer to a protocol descriptor, which provides the relative-
/// pointer equivalent to \c TargetProtocolDescriptorRef.
template <typename Runtime>
class RelativeTargetProtocolDescriptorPointer {
  union {
    /// Relative pointer to a Codira protocol descriptor.
    /// The \c bool value will be false to indicate that the protocol
    /// is a Codira protocol, or true to indicate that this references
    /// an Objective-C protocol.
    RelativeContextPointerIntPair<Runtime, bool, TargetProtocolDescriptor>
      languagePointer;
#if LANGUAGE_OBJC_INTEROP    
    /// Relative pointer to an ObjC protocol descriptor.
    /// The \c bool value will be false to indicate that the protocol
    /// is a Codira protocol, or true to indicate that this references
    /// an Objective-C protocol.
    RelativeIndirectablePointerIntPair<Protocol, bool> objcPointer;
#endif
  };

  bool isObjC() const {
#if LANGUAGE_OBJC_INTEROP
    if (Runtime::ObjCInterop)
      return objcPointer.getInt();
#endif
    return false;
  }

public:
  /// Retrieve a reference to the protocol.
  TargetProtocolDescriptorRef<Runtime> getProtocol() const {
#if LANGUAGE_OBJC_INTEROP
    if (isObjC()) {
      return TargetProtocolDescriptorRef<Runtime>::forObjC(
          const_cast<Protocol *>(objcPointer.getPointer()));
    }
#endif

    return TargetProtocolDescriptorRef<Runtime>::forCodira(
        reinterpret_cast<
            ConstTargetMetadataPointer<Runtime, TargetProtocolDescriptor>>(
            languagePointer.getPointer()));
  }

  /// Retrieve a reference to the protocol.
  int32_t getUnresolvedProtocolAddress() const {
#if LANGUAGE_OBJC_INTEROP
    if (isObjC()) {
      return objcPointer.getUnresolvedOffset();
    }
#endif
    return languagePointer.getUnresolvedOffset();
  }

  operator TargetProtocolDescriptorRef<Runtime>() const {
    return getProtocol();
  }
};

/// A reference to a type.
template <typename Runtime>
struct TargetTypeReference {
  union {
    /// A direct reference to a TypeContextDescriptor or ProtocolDescriptor.
    RelativeDirectPointer<TargetContextDescriptor<Runtime>>
      DirectTypeDescriptor;

    /// An indirect reference to a TypeContextDescriptor or ProtocolDescriptor.
    RelativeDirectPointer<
        TargetSignedPointer<Runtime, TargetContextDescriptor<Runtime> * __ptrauth_language_type_descriptor>>
      IndirectTypeDescriptor;

    /// An indirect reference to an Objective-C class.
    RelativeDirectPointer<
        ConstTargetMetadataPointer<Runtime, TargetClassMetadataType>>
      IndirectObjCClass;

    /// A direct reference to an Objective-C class name.
    RelativeDirectPointer<const char>
      DirectObjCClassName;
  };

  const TargetContextDescriptor<Runtime> *
  getTypeDescriptor(TypeReferenceKind kind) const {
    switch (kind) {
    case TypeReferenceKind::DirectTypeDescriptor:
      return DirectTypeDescriptor;

    case TypeReferenceKind::IndirectTypeDescriptor:
      return *IndirectTypeDescriptor;

    case TypeReferenceKind::DirectObjCClassName:
    case TypeReferenceKind::IndirectObjCClass:
      return nullptr;
    }

    return nullptr;
  }

  /// If this type reference is one of the kinds that supports ObjC
  /// references,
  const TargetClassMetadataObjCInterop<Runtime> *
  getObjCClass(TypeReferenceKind kind) const;

  const TargetClassMetadataObjCInterop<Runtime> * const *
  getIndirectObjCClass(TypeReferenceKind kind) const {
    assert(kind == TypeReferenceKind::IndirectObjCClass);
    return IndirectObjCClass.get();
  }

  const char *getDirectObjCClassName(TypeReferenceKind kind) const {
    assert(kind == TypeReferenceKind::DirectObjCClassName);
    return DirectObjCClassName.get();
  }
};
using TypeReference = TargetTypeReference<InProcess>;

} // end namespace language

#endif
