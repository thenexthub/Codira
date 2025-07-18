//===--- BasicBridging.h - header for the language BasicBridging module ------===//
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

#ifndef LANGUAGE_BASIC_BASICBRIDGING_H
#define LANGUAGE_BASIC_BASICBRIDGING_H

#if !defined(COMPILED_WITH_LANGUAGE) || !defined(PURE_BRIDGING_MODE)
#define USED_IN_CPP_SOURCE
#endif

// Do not add other C++/toolchain/language header files here!
// Function implementations should be placed into BasicBridging.cpp and required header files should be added there.
//
// Pure bridging mode does not permit including any C++/toolchain/language headers.
// See also the comments for `BRIDGING_MODE` in the top-level CMakeLists.txt file.
//
//
// Note: On Windows ARM64, how a C++ struct/class value type is
// returned is sensitive to conditions including whether a
// user-defined constructor exists, etc. See
// https://learn.microsoft.com/en-us/cpp/build/arm64-windows-abi-conventions?view=msvc-170#return-values
// So, if a C++ struct/class type is returned as a value between Codira
// and C++, we need to be careful to match the return convention
// matches between the non-USED_IN_CPP_SOURCE (Codira) side and the
// USE_IN_CPP_SOURCE (C++) side.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !! Do not put any constructors inside an `#ifdef USED_IN_CPP_SOURCE` block !!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include "language/Basic/BridgedCodiraObject.h"
#include "language/Basic/LanguageBridging.h"

#include <stddef.h>
#include <stdint.h>
#ifdef USED_IN_CPP_SOURCE
// Workaround to avoid a compiler error because `cas::ObjectRef` is not defined
// when including VirtualFileSystem.h
#include <cassert>
#include "toolchain/CAS/CASReference.h"

#include "language/Basic/SourceLoc.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/APInt.h"
#include <string>
#include <vector>
#endif

#ifdef PURE_BRIDGING_MODE
// In PURE_BRIDGING_MODE, briding functions are not inlined
#define BRIDGED_INLINE
#else
#define BRIDGED_INLINE inline
#endif

namespace toolchain {
class raw_ostream;
class StringRef;
class VersionTuple;
} // end namespace toolchain

namespace language {
class SourceLoc;
class SourceRange;
class CharSourceRange;
}

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

typedef intptr_t CodiraInt;
typedef uintptr_t CodiraUInt;

// Define a bridging wrapper that wraps an underlying C++ pointer type. When
// importing into Codira, we expose an initializer and accessor that works with
// `void *`, which is imported as UnsafeMutableRawPointer. Note we can't rely on
// Codira importing the underlying C++ pointer as an OpaquePointer since that is
// liable to change with PURE_BRIDGING_MODE, since that changes what we include,
// and Codira could import the underlying pointee type instead. We need to be
// careful that the interface we expose remains consistent regardless of
// PURE_BRIDGING_MODE.
#define BRIDGING_WRAPPER_IMPL(Node, Name, Qualifier, Nullability)              \
  class Bridged##Name {                                                        \
    Qualifier Node *Nullability Ptr;                                           \
                                                                               \
  public:                                                                      \
    LANGUAGE_UNAVAILABLE("Use init(raw:) instead")                                \
    Bridged##Name(Qualifier Node *Nullability ptr) : Ptr(ptr) {}               \
                                                                               \
    LANGUAGE_UNAVAILABLE("Use '.raw' instead")                                    \
    Qualifier Node *Nullability unbridged() const { return Ptr; }              \
                                                                               \
    LANGUAGE_COMPUTED_PROPERTY                                                    \
    Qualifier void *Nullability getRaw() const { return unbridged(); };        \
  };                                                                           \
                                                                               \
  LANGUAGE_NAME("Bridged" #Name ".init(raw:)")                                    \
  inline Bridged##Name Bridged##Name##_fromRaw(                                \
      Qualifier void *Nullability ptr) {                                       \
    return static_cast<Qualifier Node *>(ptr);                                 \
  }

// Bridging wrapper macros for convenience.
#define BRIDGING_WRAPPER_NONNULL(Node, Name)                                   \
  BRIDGING_WRAPPER_IMPL(Node, Name, /*unqualified*/, _Nonnull)

#define BRIDGING_WRAPPER_NULLABLE(Node, Name)                                  \
  BRIDGING_WRAPPER_IMPL(Node, Nullable##Name, /*unqualified*/, _Nullable)

#define BRIDGING_WRAPPER_CONST_NONNULL(Node, Name)                             \
  BRIDGING_WRAPPER_IMPL(Node, Name, const, _Nonnull)

#define BRIDGING_WRAPPER_CONST_NULLABLE(Node, Name)                            \
  BRIDGING_WRAPPER_IMPL(Node, Nullable##Name, const, _Nullable)

void assertFail(const char * _Nonnull msg, const char * _Nonnull file,
                CodiraUInt line, const char * _Nonnull function);

BRIDGED_INLINE
LANGUAGE_UNAVAILABLE("Unavailable in Codira")
void ASSERT_inBridgingHeader(bool condition);

//===----------------------------------------------------------------------===//
// MARK: ArrayRef
//===----------------------------------------------------------------------===//

class BridgedArrayRef {
public:
  LANGUAGE_UNAVAILABLE("Use '.data' instead")
  const void *_Nullable Data;

  LANGUAGE_UNAVAILABLE("Use '.count' instead")
  size_t Length;

  BridgedArrayRef() : Data(nullptr), Length(0) {}

  LANGUAGE_NAME("init(data:count:)")
  BridgedArrayRef(const void *_Nullable data, size_t length)
      : Data(data), Length(length) {}

#ifdef USED_IN_CPP_SOURCE
  template <typename T>
  BridgedArrayRef(toolchain::ArrayRef<T> arr)
      : Data(arr.data()), Length(arr.size()) {}

  template <typename T>
  toolchain::ArrayRef<T> unbridged() const {
    return {static_cast<const T *>(Data), Length};
  }
#endif

  LANGUAGE_COMPUTED_PROPERTY
  const void *_Nullable getData() const { return Data; }

  LANGUAGE_COMPUTED_PROPERTY
  CodiraInt getCount() const { return static_cast<CodiraInt>(Length); }

  LANGUAGE_COMPUTED_PROPERTY
  bool getIsEmpty() const { return Length == 0; }
};

//===----------------------------------------------------------------------===//
// MARK: BridgedData
//===----------------------------------------------------------------------===//

class BridgedData {
public:
  LANGUAGE_UNAVAILABLE("Use '.baseAddress' instead")
  const char *_Nullable BaseAddress;

  LANGUAGE_UNAVAILABLE("Use '.count' instead")
  size_t Length;

  BridgedData() : BaseAddress(nullptr), Length(0) {}

  LANGUAGE_NAME("init(baseAddress:count:)")
  BridgedData(const char *_Nullable baseAddress, size_t length)
      : BaseAddress(baseAddress), Length(length) {}

  LANGUAGE_COMPUTED_PROPERTY
  const char *_Nullable getBaseAddress() const { return BaseAddress; }

  LANGUAGE_COMPUTED_PROPERTY
  CodiraInt getCount() const { return static_cast<CodiraInt>(Length); }

  void free() const;
};

//===----------------------------------------------------------------------===//
// MARK: Feature
//===----------------------------------------------------------------------===//

enum ENUM_EXTENSIBILITY_ATTR(open) BridgedFeature {
#define LANGUAGE_FEATURE(FeatureName, SENumber, Description) FeatureName,
#include "language/Basic/Features.def"
};

//===----------------------------------------------------------------------===//
// MARK: StringRef
//===----------------------------------------------------------------------===//

class BridgedOStream;

class BridgedStringRef {
  const char *_Nullable Data;
  size_t Length;

public:
  BRIDGED_INLINE BridgedStringRef(toolchain::StringRef sref);
  BRIDGED_INLINE toolchain::StringRef unbridged() const;

  BridgedStringRef() : Data(nullptr), Length(0) {}

  LANGUAGE_NAME("init(data:count:)")
  BridgedStringRef(const char *_Nullable data, size_t length)
      : Data(data), Length(length) {}

  LANGUAGE_COMPUTED_PROPERTY
  const uint8_t *_Nullable getData() const { return (const uint8_t *)Data; }

  LANGUAGE_COMPUTED_PROPERTY
  CodiraInt getCount() const { return Length; }

  LANGUAGE_COMPUTED_PROPERTY
  bool getIsEmpty() const { return Length == 0; }

  void write(BridgedOStream os) const;
};

class BridgedOwnedString {
  char *_Nonnull Data;
  size_t Length;

public:
  BridgedOwnedString(toolchain::StringRef stringToCopy);
  BRIDGED_INLINE toolchain::StringRef unbridgedRef() const;

  LANGUAGE_COMPUTED_PROPERTY
  const uint8_t *_Nonnull getData() const {
    return (const uint8_t *)(Data ? Data : "");
  }

  LANGUAGE_COMPUTED_PROPERTY
  CodiraInt getCount() const { return Length; }

  LANGUAGE_COMPUTED_PROPERTY
  bool getIsEmpty() const { return Length == 0; }

  void destroy() const;
} LANGUAGE_SELF_CONTAINED;

//===----------------------------------------------------------------------===//
// MARK: BridgedOptional
//===----------------------------------------------------------------------===//

// FIXME: We should be able to make this a template once
// https://github.com/languagelang/language/issues/82258 is fixed.
#define BRIDGED_OPTIONAL(TYPE, SUFFIX)                                         \
  class LANGUAGE_CONFORMS_TO_PROTOCOL(Codira.ExpressibleByNilLiteral)              \
      BridgedOptional##SUFFIX {                                                \
    TYPE _value;                                                               \
    bool _hasValue;                                                            \
                                                                               \
  public:                                                                      \
    LANGUAGE_NAME("init(nilLiteral:)")                                            \
    BridgedOptional##SUFFIX(void) : _hasValue(false) {}                        \
    BridgedOptional##SUFFIX(TYPE value) : _value(value), _hasValue(true) {}    \
                                                                               \
    LANGUAGE_COMPUTED_PROPERTY                                                    \
    TYPE getValue() const {                                                    \
      ASSERT_inBridgingHeader(_hasValue);                                      \
      return _value;                                                           \
    }                                                                          \
                                                                               \
    LANGUAGE_COMPUTED_PROPERTY                                                    \
    bool getHasValue() const { return _hasValue; }                             \
  };
BRIDGED_OPTIONAL(CodiraInt, Int)

#ifdef USED_IN_CPP_SOURCE
inline BridgedOptionalInt getFromAPInt(toolchain::APInt i) {
  if (i.getSignificantBits() <=
      std::min(std::numeric_limits<CodiraInt>::digits, 64)) {
    return {(CodiraInt)i.getSExtValue()};
  }
  return {};
}
#endif

//===----------------------------------------------------------------------===//
// MARK: OStream
//===----------------------------------------------------------------------===//

class BridgedOStream {
  toolchain::raw_ostream * _Nonnull os;

public:
  LANGUAGE_UNAVAILABLE("Use init(raw:) instead")
  BridgedOStream(toolchain::raw_ostream * _Nonnull os) : os(os) {}

  LANGUAGE_NAME("init(raw:)")
  BridgedOStream(void *_Nonnull os)
      : os(static_cast<toolchain::raw_ostream *>(os)) {}

  LANGUAGE_UNAVAILABLE("Use '.raw' instead")
  toolchain::raw_ostream * _Nonnull unbridged() const { return os; }

  LANGUAGE_COMPUTED_PROPERTY
  void *_Nonnull getRaw(BridgedOStream bridged) const { return unbridged(); }

  void write(BridgedStringRef string) const;

  void newLine() const;

  void flush() const;
};

BridgedOStream Bridged_dbgs();

//===----------------------------------------------------------------------===//
// MARK: SourceLoc
//===----------------------------------------------------------------------===//

class BridgedSourceLoc {
  const void *_Nullable Raw;

public:
  BridgedSourceLoc() : Raw(nullptr) {}

  LANGUAGE_NAME("init(raw:)")
  BridgedSourceLoc(const void *_Nullable raw) : Raw(raw) {}

  BRIDGED_INLINE BridgedSourceLoc(language::SourceLoc loc);

  BRIDGED_INLINE language::SourceLoc unbridged() const;

  LANGUAGE_IMPORT_UNSAFE
  const void *_Nullable getOpaquePointerValue() const { return Raw; }

  LANGUAGE_COMPUTED_PROPERTY
  bool getIsValid() const { return Raw != nullptr; }

  LANGUAGE_NAME("advanced(by:)")
  BRIDGED_INLINE
  BridgedSourceLoc advancedBy(size_t n) const;
};

//===----------------------------------------------------------------------===//
// MARK: SourceRange
//===----------------------------------------------------------------------===//

class BridgedSourceRange {
public:
  LANGUAGE_NAME("start")
  BridgedSourceLoc Start;

  LANGUAGE_NAME("end")
  BridgedSourceLoc End;

  BridgedSourceRange() : Start(), End() {}

  LANGUAGE_NAME("init(start:end:)")
  BridgedSourceRange(BridgedSourceLoc start, BridgedSourceLoc end)
      : Start(start), End(end) {}

  BRIDGED_INLINE BridgedSourceRange(language::SourceRange range);

  BRIDGED_INLINE language::SourceRange unbridged() const;
};

//===----------------------------------------------------------------------===//
// MARK: BridgedCharSourceRange
//===----------------------------------------------------------------------===//

class BridgedCharSourceRange {
public:
  LANGUAGE_UNAVAILABLE("Use '.start' instead")
  BridgedSourceLoc Start;

  LANGUAGE_UNAVAILABLE("Use '.byteLength' instead")
  unsigned ByteLength;

  LANGUAGE_NAME("init(start:byteLength:)")
  BridgedCharSourceRange(BridgedSourceLoc start, unsigned byteLength)
      : Start(start), ByteLength(byteLength) {}

  BRIDGED_INLINE BridgedCharSourceRange(language::CharSourceRange range);

  BRIDGED_INLINE language::CharSourceRange unbridged() const;

  LANGUAGE_COMPUTED_PROPERTY
  BridgedSourceLoc getStart() const { return Start; }

  LANGUAGE_COMPUTED_PROPERTY
  CodiraInt getByteLength() const { return static_cast<CodiraInt>(ByteLength); }
};

//===----------------------------------------------------------------------===//
// MARK: std::vector<BridgedCharSourceRange>
//===----------------------------------------------------------------------===//

/// An opaque, heap-allocated `std::vector<CharSourceRange>`.
///
/// This type is manually memory managed. The creator of the object needs to
/// ensure that `takeUnbridged` is called to free the memory.
class BridgedCharSourceRangeVector {
  /// Opaque pointer to `std::vector<CharSourceRange>`.
  void *_Nonnull vector;

public:
  BridgedCharSourceRangeVector();

  LANGUAGE_NAME("append(_:)")
  void push_back(BridgedCharSourceRange range);

#ifdef USED_IN_CPP_SOURCE
  /// Returns the `std::vector<language::CharSourceRange>` that this
  /// `BridgedCharSourceRangeVector` represents and frees the memory owned by
  /// this `BridgedCharSourceRangeVector`.
  ///
  /// No operations should be called on `BridgedCharSourceRangeVector` after
  /// `takeUnbridged` is called.
  std::vector<language::CharSourceRange> takeUnbridged() {
    auto *vectorPtr =
        static_cast<std::vector<language::CharSourceRange> *>(vector);
    std::vector<language::CharSourceRange> unbridged = *vectorPtr;
    delete vectorPtr;
    return unbridged;
  }
#endif
};

//===----------------------------------------------------------------------===//
// MARK: BridgedCodiraVersion
//===----------------------------------------------------------------------===//

class BridgedCodiraVersion {
  unsigned Major;
  unsigned Minor;

public:
  BridgedCodiraVersion() : Major(0), Minor(0) {}

  BRIDGED_INLINE
  LANGUAGE_NAME("init(major:minor:)")
  BridgedCodiraVersion(CodiraInt major, CodiraInt minor);

  unsigned getMajor() const { return Major; }
  unsigned getMinor() const { return Minor; }
};

//===----------------------------------------------------------------------===//
// MARK: GeneratedSourceInfo
//===----------------------------------------------------------------------===//

enum ENUM_EXTENSIBILITY_ATTR(closed) BridgedGeneratedSourceFileKind {
#define MACRO_ROLE(Name, Description)                                          \
  BridgedGeneratedSourceFileKind##Name##MacroExpansion,
#include "language/Basic/MacroRoles.def"
#undef MACRO_ROLE

  BridgedGeneratedSourceFileKindReplacedFunctionBody,
  BridgedGeneratedSourceFileKindPrettyPrinted,
  BridgedGeneratedSourceFileKindDefaultArgument,
  BridgedGeneratedSourceFileKindAttributeFromClang,

  BridgedGeneratedSourceFileKindNone,
};

//===----------------------------------------------------------------------===//
// MARK: VirtualFile
//===----------------------------------------------------------------------===//

struct BridgedVirtualFile {
  size_t StartPosition;
  size_t EndPosition;
  BridgedStringRef Name;
  ptrdiff_t LineOffset;
  size_t NamePosition;
};

//===----------------------------------------------------------------------===//
// MARK: VersionTuple
//===----------------------------------------------------------------------===//

struct BridgedVersionTuple {
  unsigned Major : 32;

  unsigned Minor : 31;
  unsigned HasMinor : 1;

  unsigned Subminor : 31;
  unsigned HasSubminor : 1;

  unsigned Build : 31;
  unsigned HasBuild : 1;

  BridgedVersionTuple()
      : Major(0), Minor(0), HasMinor(false), Subminor(0), HasSubminor(false),
        Build(0), HasBuild(false) {}
  BridgedVersionTuple(unsigned Major)
      : Major(Major), Minor(0), HasMinor(false), Subminor(0),
        HasSubminor(false), Build(0), HasBuild(false) {}
  BridgedVersionTuple(unsigned Major, unsigned Minor)
      : Major(Major), Minor(Minor), HasMinor(true), Subminor(0),
        HasSubminor(false), Build(0), HasBuild(false) {}
  BridgedVersionTuple(unsigned Major, unsigned Minor, unsigned Subminor)
      : Major(Major), Minor(Minor), HasMinor(true), Subminor(Subminor),
        HasSubminor(true), Build(0), HasBuild(false) {}
  BridgedVersionTuple(unsigned Major, unsigned Minor, unsigned Subminor,
                      unsigned Build)
      : Major(Major), Minor(Minor), HasMinor(true), Subminor(Subminor),
        HasSubminor(true), Build(Build), HasBuild(true) {}

  BridgedVersionTuple(toolchain::VersionTuple);

  toolchain::VersionTuple unbridged() const;
};

//===----------------------------------------------------------------------===//
// MARK: BridgedCodiraClosure
//===----------------------------------------------------------------------===//

/// Wrapping a pointer to a Codira closure `(UnsafeRawPointer?) -> Void`
/// See 'withBridgedCodiraClosure(closure:call:)' in ASTGen.
struct BridgedCodiraClosure {
  const void *_Nonnull closure;

  BRIDGED_INLINE void operator()(const void *_Nullable);
};

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#ifndef PURE_BRIDGING_MODE
// In _not_ PURE_BRIDGING_MODE, bridging functions are inlined and therefore
// included in the header file. This is because they rely on C++ headers that
// we don't want to pull in when using "pure bridging mode".
#include "BasicBridgingImpl.h"
#endif

#endif
