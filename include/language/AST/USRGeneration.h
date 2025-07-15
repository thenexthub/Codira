//===--- USRGeneration.h - Routines for USR generation ----------*- C++ -*-===//
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
// Unique Symbol References (USRs) provide a textual encoding for
// declarations. These are used for indexing, analogous to how mangled names
// are used in object files.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_USRGENERATION_H
#define LANGUAGE_AST_USRGENERATION_H

#include "language/Basic/Toolchain.h"

#include <string>

namespace language {
class Decl;
class AbstractStorageDecl;
class ValueDecl;
class ExtensionDecl;
class ModuleEntity;
enum class AccessorKind;
class Type;

namespace ide {

/// Prints out the USR for the Type.
/// \returns true if it failed, false on success.
bool printTypeUSR(Type Ty, raw_ostream &OS);

/// Prints out the USR for the Type of the given decl.
/// \returns true if it failed, false on success.
bool printDeclTypeUSR(const ValueDecl *D, raw_ostream &OS);

/// Prints out the USR for the given ValueDecl.
/// @param distinguishSynthesizedDecls Whether to use the USR of the
/// synthesized declaration instead of the USR of the underlying Clang USR.
/// \returns true if it failed, false on success.
bool printValueDeclUSR(const ValueDecl *D, raw_ostream &OS,
                       bool distinguishSynthesizedDecls = false);

/// Prints out the USR for the given ModuleEntity.
/// In case module aliasing is used, it prints the real module name. For example,
/// if a file has `import Foo` and `-module-alias Foo=Bar` is passed, treat Foo as
/// an alias and Bar as the real module name as its dependency. Note that the
/// aliasing only applies to Codira modules.
/// \returns true if it failed, false on success.
bool printModuleUSR(ModuleEntity Mod, raw_ostream &OS);

/// Prints out the accessor USR for the given storage Decl.
/// \returns true if it failed, false on success.
bool printAccessorUSR(const AbstractStorageDecl *D, AccessorKind AccKind,
                      toolchain::raw_ostream &OS);

/// Prints out the extension USR for the given extension Decl.
/// \returns true if it failed, false on success.
bool printExtensionUSR(const ExtensionDecl *ED, raw_ostream &OS);

/// Prints out the USR for the given Decl.
/// @param distinguishSynthesizedDecls Whether to use the USR of the
/// synthesized declaration instead of the USR of the underlying Clang USR.
/// \returns true if it failed, false on success.
bool printDeclUSR(const Decl *D, raw_ostream &OS,
                  bool distinguishSynthesizedDecls = false);

/// Demangle a mangle-name-based USR to a human readable name.
std::string demangleUSR(StringRef mangled);

} // namespace ide
} // namespace language

#endif // TOOLCHAIN_LANGUAGE_AST_USRGENERATION_H

