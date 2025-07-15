//===--- KnownMetadata.cpp - Codira Language ABI Known Metadata Objects ----===//
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
// Definitions of some builtin metadata objects.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Metadata.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Numeric.h"
#include "MetadataImpl.h"
#include "Private.h"
#include <cstring>
#include <climits>

namespace language {
class AsyncTask;
class Job;
}

using namespace language;
using namespace metadataimpl;

/// Copy a value from one object to another based on the size in the
/// given type metadata.
OpaqueValue *language::language_copyPOD(OpaqueValue *dest, OpaqueValue *src,
                                  const Metadata *type) {
  return (OpaqueValue*) memcpy(dest, src, type->getValueWitnesses()->size);
}

namespace {
  // A type sized and aligned the way Codira wants Int128 (and Float80/Float128)
  // to be sized and aligned.
  struct alignas(16) int128_like {
    char data[16];
  };

  static_assert(MaximumAlignment == 16, "max alignment was hardcoded");
  struct alignas(16) int256_like {
    char data[32];
  };
  struct alignas(16) int512_like {
    char data[64];
  };

  struct alignas(16) float80_like {
    char data[10];
  };
} // end anonymous namespace

namespace ctypes {
  namespace {
    // Type definitions that map each names Codira builtin type to their
    // C counterparts.
    using Bi1_ = uint8_t;
    using Bi7_ = uint8_t;
    using Bi8_ = uint8_t;
    using Bi16_ = uint16_t;
    using Bi32_ = uint32_t;
    using Bi63_ = uint64_t;
    using Bi64_ = uint64_t;
    using Bi128_ = int128_like;
    using Bi256_ = int256_like;
    using Bi512_ = int512_like;

    using Bw = intptr_t;
    using BL_ = IntegerLiteral;

    using Bf16_ = uint16_t;
    using Bf32_ = float;
    using Bf64_ = double;
    using Bf80_ = float80_like;
    using Bf128_ = int128_like;

    /// The value-witness table for UnsafeValueBuffer.  You can do layout
    /// with this, but the type isn't copyable, so most of the value
    /// operations are meaningless.
    using BB = ValueBuffer;

    // Types that are defined in the _Concurrency library

    // Default actor storage type.
    struct alignas(2 * alignof(void*)) BD {
      void *storage[NumWords_DefaultActor];
    };

    // SerialExecutorRef type.
    struct Be {
      HeapObject *Identity;
      uintptr_t Implementation;
    };

    // Types that are defined in the Distributed library

    // Non-default distributed actor storage type.
    struct alignas(2 * alignof(void*)) Bd {
      void *storage[NumWords_NonDefaultDistributedActor];
    };
  }
}

namespace pointer_types {
  namespace {
    /// The basic value-witness table for Codira object pointers.
    using Bo = CodiraRetainableBox;

    /// The value-witness table for raw pointers.
    using Bp = RawPointerBox;

    /// The value-witness table for BridgeObject.
    using Bb = BridgeObjectBox;

    // RawUnsafeContinuation type.
    using Bc = RawPointerBox;

    // Job type.
    using Bj = RawPointerBox;

#if LANGUAGE_OBJC_INTEROP
    /*** Objective-C pointers *************************************************/

    // This section can reasonably be suppressed in builds that don't
    // need to support Objective-C.

    /// The basic value-witness table for ObjC object pointers.
    using BO = ObjCRetainableBox;
#else
    using BO = UnknownObjectRetainableBox;
#endif

  }
}

namespace {
  template <typename T>
  struct BuiltinType {
    static constexpr const size_t Alignment = alignof(T);
  };

#define SET_FIXED_ALIGNMENT(Type, Align)                                       \
  template <>                                                                  \
  struct BuiltinType<Type> {                                                   \
    static constexpr const size_t Alignment = Align;                           \
  };

  SET_FIXED_ALIGNMENT(uint8_t, 1)
  SET_FIXED_ALIGNMENT(uint16_t, 2)
  SET_FIXED_ALIGNMENT(uint32_t, 4)
  SET_FIXED_ALIGNMENT(uint64_t, 8)
  SET_FIXED_ALIGNMENT(int128_like, 16)
  static_assert(MaximumAlignment == 16, "max alignment was hardcoded");
  SET_FIXED_ALIGNMENT(int256_like, 16)
  SET_FIXED_ALIGNMENT(int512_like, 16)

#undef SET_FIXED_ALIGNMENT

  template <typename T, unsigned N>
  struct SIMDVector {
    using Type = T __attribute__((__ext_vector_type__(N)));
  };

  template <>
  struct SIMDVector<float, 3> {
    using Type = float __attribute__((__ext_vector_type__(4)));
  };

  template <>
  struct SIMDVector<double, 3> {
    using Type = double __attribute__((__ext_vector_type__(4)));
  };

  template <typename T, unsigned N>
  using SIMDVectorType = typename SIMDVector<T, N>::Type;
}

#define BUILTIN_TYPE(Symbol, Name)                                             \
const ValueWitnessTable language::VALUE_WITNESS_SYM(Symbol) =                     \
  ValueWitnessTableForBox<NativeBox<ctypes::Symbol,                            \
                                    BuiltinType<ctypes::Symbol>::Alignment>>::table;

#define BUILTIN_POINTER_TYPE(Symbol, Name)                 \
const ValueWitnessTable language::VALUE_WITNESS_SYM(Symbol) =     \
  ValueWitnessTableForBox<pointer_types::Symbol>::table;

#if LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES
#define BUILTIN_VECTOR_TYPE(ElementSymbol, _, Width)                           \
  const ValueWitnessTable                                                      \
  language::VALUE_WITNESS_SYM(VECTOR_BUILTIN_SYMBOL_NAME(ElementSymbol,Width)) =  \
  ValueWitnessTableForBox<NativeBox<SIMDVectorType<ctypes::ElementSymbol,      \
                                                   Width>>>::table;
#else
#define BUILTIN_VECTOR_TYPE(ElementSymbol, ElementName, Width)
#endif

#include "language/Runtime/BuiltinTypes.def"

/// The value-witness table for pointer-aligned unmanaged pointer types.
const ValueWitnessTable language::METATYPE_VALUE_WITNESS_SYM(Bo) =
  ValueWitnessTableForBox<PointerPointerBox>::table;

/*** Functions ***************************************************************/

namespace {
  // @escaping function types.
  struct ThickFunctionBox
    : AggregateBox<FunctionPointerBox, CodiraRetainableBox> {

    static constexpr unsigned numExtraInhabitants =
      FunctionPointerBox::numExtraInhabitants;

    static void storeExtraInhabitantTag(char *dest, unsigned tag) {
      FunctionPointerBox::storeExtraInhabitantTag((void**) dest, tag);
    }

    static unsigned getExtraInhabitantTag(const char *src) {
      return FunctionPointerBox::getExtraInhabitantTag((void * const *) src);
    }
  };
  /// @noescape function types.
  struct TrivialThickFunctionBox
      : AggregateBox<FunctionPointerBox, RawPointerBox> {

    static constexpr unsigned numExtraInhabitants =
        FunctionPointerBox::numExtraInhabitants;

    static void storeExtraInhabitantTag(char *dest, unsigned tag) {
      FunctionPointerBox::storeExtraInhabitantTag((void **)dest, tag);
    }

    static unsigned getExtraInhabitantTag(const char *src) {
      return FunctionPointerBox::getExtraInhabitantTag((void *const *)src);
    }
  };
  struct DiffFunctionBox
    : AggregateBox<ThickFunctionBox, ThickFunctionBox, ThickFunctionBox> {

    static constexpr unsigned numExtraInhabitants =
      ThickFunctionBox::numExtraInhabitants;

    static void storeExtraInhabitantTag(char *dest, unsigned tag) {
      ThickFunctionBox::storeExtraInhabitantTag(dest, tag);
    }

    static unsigned getExtraInhabitantTag(const char *src) {
      return ThickFunctionBox::getExtraInhabitantTag(src);
    }
  };
} // end anonymous namespace

/// The basic value-witness table for escaping function types.
const ValueWitnessTable
  language::VALUE_WITNESS_SYM(FUNCTION_MANGLING) =
    ValueWitnessTableForBox<ThickFunctionBox>::table;

const ValueWitnessTable
  language::VALUE_WITNESS_SYM(DIFF_FUNCTION_MANGLING) =
    ValueWitnessTableForBox<DiffFunctionBox>::table;

/// The basic value-witness table for @noescape function types.
const ValueWitnessTable
  language::VALUE_WITNESS_SYM(NOESCAPE_FUNCTION_MANGLING) =
    ValueWitnessTableForBox<TrivialThickFunctionBox>::table;

/// The basic value-witness table for thin function types.
const ValueWitnessTable
  language::VALUE_WITNESS_SYM(THIN_FUNCTION_MANGLING) =
    ValueWitnessTableForBox<FunctionPointerBox>::table;

/*** Empty tuples ************************************************************/

/// The basic value-witness table for empty types.
const ValueWitnessTable language::VALUE_WITNESS_SYM(EMPTY_TUPLE_MANGLING) =
  ValueWitnessTableForBox<AggregateBox<>>::table;

/*** Known metadata **********************************************************/

// Define some builtin opaque metadata.
#define OPAQUE_METADATA(TYPE) \
  const FullOpaqueMetadata language::METADATA_SYM(TYPE) = { \
    { &VALUE_WITNESS_SYM(TYPE) },                             \
    { { MetadataKind::Opaque } }                 \
  };
#define BUILTIN_TYPE(Symbol, Name) \
  OPAQUE_METADATA(Symbol)
#if !LANGUAGE_STDLIB_ENABLE_VECTOR_TYPES
#define BUILTIN_VECTOR_TYPE(ElementSymbol, ElementName, Width)
#endif
#include "language/Runtime/BuiltinTypes.def"

/// The standard metadata for the empty tuple.
const FullMetadata<TupleTypeMetadata> language::
METADATA_SYM(EMPTY_TUPLE_MANGLING) = {
  { &VALUE_WITNESS_SYM(EMPTY_TUPLE_MANGLING) },                 // ValueWitnesses
  {
    { MetadataKind::Tuple },   // Kind
    0,                         // NumElements
    nullptr                    // Labels
  }
};

