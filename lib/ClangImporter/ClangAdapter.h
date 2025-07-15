//===--- ClangAdapter.h - Interfaces with Clang entities --------*- C++ -*-===//
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
// This file provides convenient and canonical interfaces with Clang entities,
// serving as both a useful place to put utility functions and a canonical
// interface that can abstract nitty gritty Clang internal details.
//
//===----------------------------------------------------------------------===//
#ifndef CLANG_ADAPTER_H
#define CLANG_ADAPTER_H

#include "language/Basic/StringExtras.h"
#include "clang/Basic/Specifiers.h"
#include "toolchain/ADT/SmallBitVector.h"
#include <optional>

#include "ImportName.h"

namespace clang {
class ASTContext;
class Decl;
class DeclContext;
class MacroInfo;
class Module;
class NamedDecl;
class ObjCInterfaceDecl;
class ObjCMethodDecl;
class ObjCPropertyDecl;
class ParmVarDecl;
class QualType;
class Sema;
class CodiraNewTypeAttr;
class Type;
class TypedefNameDecl;
}

// TODO: pull more off of the ImportImpl

namespace language {
enum OptionalTypeKind : unsigned;

namespace importer {
struct PlatformAvailability;

/// Returns the redeclaration of \p D that contains its definition for any
/// tag type decl (struct, enum, or union) or Objective-C class or protocol.
///
/// Returns \c None if \p D is not a redeclarable type declaration.
/// Returns null if \p D is a redeclarable type, but it does not have a
/// definition yet.
std::optional<const clang::Decl *>
getDefinitionForClangTypeDecl(const clang::Decl *D);

/// Returns the first redeclaration of \p D outside of a function.
///
/// C allows redeclaring most declarations in function bodies, as so:
///
///     void usefulPublicFunction(void) {
///       extern void importantInternalFunction(int code);
///       importantInternalFunction(42);
///     }
///
/// This should allow clients to call \c usefulPublicFunction without exposing
/// \c importantInternalFunction . However, if there is another declaration of
/// \c importantInternalFunction later, Clang still needs to treat them as the
/// same function. This is normally fine...except that if the local declaration
/// is the \e first declaration, it'll also get used as the "canonical"
/// declaration that Clang (and Codira) use for uniquing purposes.
///
/// Every imported declaration gets assigned to a module in Codira, and for
/// declarations without definitions that choice is somewhat arbitrary. But it
/// would be better not to pick a local declaration like the one above, and
/// therefore this method should be used instead of
/// clang::Decl::getCanonicalDecl when the containing module is important.
///
/// If there are no non-local redeclarations, returns null.
/// If \p D is not a kind of declaration that supports being redeclared, just
/// returns \p D itself.
const clang::Decl *
getFirstNonLocalDecl(const clang::Decl *D);

/// Returns the module \p D comes from, or \c None if \p D does not have
/// a valid associated module.
///
/// The returned module may be null (but not \c None) if \p D comes from
/// an imported header.
std::optional<clang::Module *>
getClangSubmoduleForDecl(const clang::Decl *D,
                         bool allowForwardDeclaration = false);

/// Retrieve the type of an instance of the given Clang declaration context,
/// or a null type if the DeclContext does not have a corresponding type.
clang::QualType getClangDeclContextType(const clang::DeclContext *dc);

/// Retrieve the type name of a Clang type for the purposes of
/// omitting unneeded words.
OmissionTypeName getClangTypeNameForOmission(clang::ASTContext &ctx,
                                             clang::QualType type);

/// Find the language_newtype attribute on the given typedef, if present.
clang::CodiraNewTypeAttr *getCodiraNewtypeAttr(const clang::TypedefNameDecl *decl,
                                             ImportNameVersion version);

/// Retrieve a bit vector containing the non-null argument
/// annotations for the given declaration.
SmallBitVector
getNonNullArgs(const clang::Decl *decl,
               ArrayRef<const clang::ParmVarDecl *> params);

/// Whether the given decl is a global Notification
bool isNSNotificationGlobal(const clang::NamedDecl *);

// If this decl is associated with a language_newtype (and we're honoring
// language_newtype), return it, otherwise null
clang::TypedefNameDecl *findCodiraNewtype(const clang::NamedDecl *decl,
                                         clang::Sema &clangSema,
                                         ImportNameVersion version);

/// Whether the passed type is NSString *
bool isNSString(const clang::Type *);
bool isNSString(clang::QualType);

/// Wehther the passed type is `NSNotificationName` typealias
bool isNSNotificationName(clang::QualType);

/// Whether the given declaration was exported from Codira.
///
/// Note that this only checks the immediate declaration being passed.
/// For things like methods and properties that are nested in larger types,
/// it's the top-level declaration that should be checked.
bool hasNativeCodiraDecl(const clang::Decl *decl);

/// Translation API nullability from an API note into an optional kind.
///
/// \param stripNonResultOptionality Whether strip optionality from
/// \c _Nullable but not \c _Nullable_result.
OptionalTypeKind translateNullability(
    clang::NullabilityKind kind, bool stripNonResultOptionality = false);

/// Determine whether the given method is a required initializer
/// of the given class.
bool isRequiredInitializer(const clang::ObjCMethodDecl *method);

/// Determine whether this property should be imported as its getter and setter
/// rather than as a Codira property.
bool shouldImportPropertyAsAccessors(const clang::ObjCPropertyDecl *prop);

/// Determine whether this method is an Objective-C "init" method
/// that will be imported as a Codira initializer.
bool isInitMethod(const clang::ObjCMethodDecl *method);

/// Determine whether this is the declaration of Objective-C's 'id' type.
bool isObjCId(const clang::Decl *decl);

/// Determine whether the given declaration is considered
/// 'unavailable' in Codira.
bool isUnavailableInCodira(const clang::Decl *decl, const PlatformAvailability *,
                          bool enableObjCInterop);

/// Determine the optionality of the given Clang parameter.
///
/// \param param The Clang parameter.
///
/// \param knownNonNull Whether a function- or method-level "nonnull" attribute
/// applies to this parameter.
OptionalTypeKind getParamOptionality(const clang::ParmVarDecl *param,
                                     bool knownNonNull);
}
}

#endif
