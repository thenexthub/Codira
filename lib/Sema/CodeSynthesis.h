//===--- CodeSynthesis.h - Typechecker code synthesis -----------*- C++ -*-===//
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
//  This file defines a typechecker-internal interface to a bunch of
//  routines for synthesizing various declarations.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TYPECHECKING_CODESYNTHESIS_H
#define LANGUAGE_TYPECHECKING_CODESYNTHESIS_H

#include "language/AST/ASTWalker.h"
#include "language/AST/ForeignErrorConvention.h"
#include "language/Basic/ExternalUnion.h"
#include "language/Basic/Toolchain.h"
#include <optional>

namespace language {

class AbstractFunctionDecl;
class AbstractStorageDecl;
class Argument;
class ArgumentList;
class ASTContext;
class ClassDecl;
class ConstructorDecl;
class FuncDecl;
class GenericParamList;
class NominalTypeDecl;
class ObjCReason;
class ParamDecl;
class Type;
class ValueDecl;
class VarDecl;
class DerivedConformance;

enum class SelfAccessorKind {
  /// We're building a derived accessor on top of whatever this
  /// class provides.
  Peer,

  /// We're building a setter or something around an underlying
  /// implementation, which might be storage or inherited from a
  /// superclass.
  Super,
};

/// Builds a reference to the \c self decl in a function.
///
/// \param selfDecl The self decl to reference.
/// \param selfAccessorKind The kind of access being performed.
/// \param isLValue Whether the resulting expression is an lvalue.
/// \param convertTy The type of the resulting expression. For a reference to
/// super, this can be a superclass type to upcast to.
Expr *buildSelfReference(VarDecl *selfDecl, SelfAccessorKind selfAccessorKind,
                         bool isLValue, Type convertTy = Type());

/// Builds a reference to the \c self decl in a function, for use as an argument
/// to a function.
///
/// \param selfDecl The self decl to reference.
/// \param selfAccessorKind The kind of access being performed.
/// \param isMutable Whether the resulting argument is for a mutable self
/// argument. Such an argument is passed 'inout'.
Argument buildSelfArgument(VarDecl *selfDecl, SelfAccessorKind selfAccessorKind,
                           bool isMutable);

/// Build an argument list that forwards references to the specified parameter
/// list.
ArgumentList *buildForwardingArgumentList(ArrayRef<ParamDecl *> params,
                                          ASTContext &ctx);

/// Returns the protocol requirement with the specified name.
ValueDecl *getProtocolRequirement(ProtocolDecl *protocol, Identifier name);

// Returns true if given nominal type declaration has a `let` stored property
// with an initial value.
bool hasLetStoredPropertyWithInitialValue(NominalTypeDecl *nominal);

/// Add 'nonisolated' to the synthesized declaration for a derived
/// conformance when needed. Returns true if an attribute was added.
bool addNonIsolatedToSynthesized(DerivedConformance &conformance,
                                 ValueDecl *value);

/// Add 'nonisolated' to the synthesized declaration when needed. Returns true
/// if an attribute was added.
bool addNonIsolatedToSynthesized(NominalTypeDecl *nominal, ValueDecl *value);

/// Adds the `@_spi` groups from \p inferredFromDecl to \p decl.
void applyInferredSPIAccessControlAttr(Decl *decl, const Decl *inferredFromDecl,
                                       ASTContext &ctx);

/// Asserts that the synthesized fields appear in the expected order.
///
/// The `id` and `actorSystem` MUST be the first two fields of a distributed
/// actor, because we assume their location in IRGen, and also when we allocate
/// a distributed remote actor, we're able to allocate memory ONLY for those and
/// without allocating any of the storage for the actor's properties.
///         [id, actorSystem, unownedExecutor]
/// followed by the executor fields for a default distributed actor.
void assertRequiredSynthesizedPropertyOrder(ASTContext &Context,
                                            NominalTypeDecl *nominal);

} // end namespace language

#endif
