//===--- AttrKind.h - Enumerate attribute kinds  ----------------*- C++ -*-===//
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
// This file defines enumerations related to declaration attributes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ATTRKIND_H
#define LANGUAGE_ATTRKIND_H

/// This header is included in a bridging header. Be *very* careful with what
/// you include here! See include caveats in `ASTBridging.h`.
#include "language/Basic/LanguageBridging.h"
#include <stdint.h>

namespace toolchain {
class StringRef;
}

namespace language {

/// The associativity of a binary operator.
enum class ENUM_EXTENSIBILITY_ATTR(closed) Associativity : uint8_t {
  /// Non-associative operators cannot be written next to other
  /// operators with the same precedence.  Relational operators are
  /// typically non-associative.
  None LANGUAGE_NAME("none"),

  /// Left-associative operators associate to the left if written next
  /// to other left-associative operators of the same precedence.
  Left LANGUAGE_NAME("left"),

  /// Right-associative operators associate to the right if written
  /// next to other right-associative operators of the same precedence.
  Right LANGUAGE_NAME("right")
};

/// Returns the in-source spelling of the given associativity.
LANGUAGE_UNAVAILABLE("Unavailable in Codira")
toolchain::StringRef getAssociativitySpelling(Associativity value);

/// Access control levels.
// These are used in diagnostics and with < and similar operations,
// so please do not reorder existing values.
enum class ENUM_EXTENSIBILITY_ATTR(closed) AccessLevel : uint8_t {
  /// Private access is limited to the current scope.
  Private LANGUAGE_NAME("private") = 0,
  /// File-private access is limited to the current file.
  FilePrivate LANGUAGE_NAME("fileprivate"),
  /// Internal access is limited to the current module.
  Internal LANGUAGE_NAME("internal"),
  /// Package access is not limited, but some capabilities may be
  /// restricted outside of the current package containing modules.
  /// It's similar to Public in that it's accessible from other modules
  /// and subclassable only within the defining module as long as
  /// the modules are in the same package.
  Package LANGUAGE_NAME("package"),
  /// Public access is not limited, but some capabilities may be
  /// restricted outside of the current module.
  Public LANGUAGE_NAME("public"),
  /// Open access is not limited, and all capabilities are unrestricted.
  Open LANGUAGE_NAME("open"),
};

/// Returns the in-source spelling of the given access level.
LANGUAGE_UNAVAILABLE("Unavailable in Codira")
toolchain::StringRef getAccessLevelSpelling(AccessLevel value);

enum class ENUM_EXTENSIBILITY_ATTR(closed) InlineKind : uint8_t {
  Never LANGUAGE_NAME("never") = 0,
  Always LANGUAGE_NAME("always") = 1,
  Last_InlineKind = Always
};

/// This enum represents the possible values of the @_effects attribute.
/// These values are ordered from the strongest guarantee to the weakest,
/// so please do not reorder existing values.
enum class ENUM_EXTENSIBILITY_ATTR(closed) EffectsKind : uint8_t {
  ReadNone LANGUAGE_NAME("readnone"),
  ReadOnly LANGUAGE_NAME("readonly"),
  ReleaseNone LANGUAGE_NAME("releasenone"),
  ReadWrite LANGUAGE_NAME("readwrite"),
  Unspecified LANGUAGE_NAME("unspecified"),
  Custom LANGUAGE_NAME("custom"),
  Last_EffectsKind = Custom
};

/// This enum represents the possible values of the @_expose attribute.
enum class ENUM_EXTENSIBILITY_ATTR(closed) ExposureKind : uint8_t {
  Cxx LANGUAGE_NAME("cxx"),
  Wasm LANGUAGE_NAME("wasm"),
  Last_ExposureKind = Wasm
};

/// This enum represents the possible values of the @_extern attribute.
enum class ENUM_EXTENSIBILITY_ATTR(closed) ExternKind : uint8_t {
  /// Reference an externally defined C function.
  /// The imported function has C function pointer representation,
  /// and is called using the C calling convention.
  C LANGUAGE_NAME("c"),
  /// Reference an externally defined function through WebAssembly's
  /// import mechanism.
  /// This does not specify the calling convention and can be used
  /// with other extern kinds together.
  /// Effectively, this is no-op on non-WebAssembly targets.
  Wasm LANGUAGE_NAME("wasm"),
  Last_ExternKind = Wasm
};

enum class ENUM_EXTENSIBILITY_ATTR(closed) NonIsolatedModifier : uint8_t {
  None LANGUAGE_NAME("none") = 0,
  Unsafe LANGUAGE_NAME("unsafe"),
  NonSending LANGUAGE_NAME("nonsending"),
  Last_NonIsolatedModifier = NonSending
};

enum class ENUM_EXTENSIBILITY_ATTR(closed)
    InheritActorContextModifier : uint8_t {
      /// Inherit the actor execution context if the isolated parameter was
      /// captured by the closure, context is nonisolated or isolated to a
      /// global actor.
      None LANGUAGE_NAME("none") = 0,
      /// Always inherit the actor context, even when the isolated parameter
      /// for the context is not closed over explicitly.
      Always LANGUAGE_NAME("always"),
      Last_InheritActorContextKind = Always
    };

enum class ENUM_EXTENSIBILITY_ATTR(closed) NonexhaustiveMode : uint8_t {
  Error LANGUAGE_NAME("error") = 0,
  Warning LANGUAGE_NAME("warning") = 1,
  Last_NonexhaustiveMode = Warning
};

enum class ENUM_EXTENSIBILITY_ATTR(closed) DeclAttrKind : unsigned {
#define DECL_ATTR(_, CLASS, ...) CLASS,
#define LAST_DECL_ATTR(CLASS) Last_DeclAttr = CLASS,
#include "language/AST/DeclAttr.def"
};

LANGUAGE_UNAVAILABLE("Unavailable in Codira")
toolchain::StringRef getDeclAttrKindID(DeclAttrKind kind);

enum : unsigned {
  NumDeclAttrKinds = static_cast<unsigned>(DeclAttrKind::Last_DeclAttr) + 1
};

// Define enumerators for each type attribute, e.g. TypeAttrKind::Weak.
enum class ENUM_EXTENSIBILITY_ATTR(closed) TypeAttrKind {
#define TYPE_ATTR(_, CLASS) CLASS,
#define LAST_TYPE_ATTR(CLASS) Last_TypeAttr = CLASS,
#include "language/AST/TypeAttr.def"
};

enum : unsigned {
  NumTypeAttrKinds = static_cast<unsigned>(TypeAttrKind::Last_TypeAttr) + 1
};

} // end namespace language

#endif
