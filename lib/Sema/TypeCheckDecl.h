//===--- TypeCheckDecl.h ----------------------------------------*- C++ -*-===//
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
//  routines for semantic checking of declaration.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TYPECHECKING_TYPECHECKDECL_H
#define LANGUAGE_TYPECHECKING_TYPECHECKDECL_H

#include <optional>

namespace language {

class ASTContext;
class DeclContext;
class ValueDecl;
class Pattern;
class ConstructorDecl;
class EnumDecl;
class SourceFile;
class PrecedenceGroupDecl;
class ParameterList;

/// Walks up the override chain for \p CD until it finds an initializer that is
/// required and non-implicit. If no such initializer exists, returns the
/// declaration where \c required was introduced (i.e. closest to the root
/// class).
const ConstructorDecl *findNonImplicitRequiredInit(const ConstructorDecl *CD);

// Implemented in TypeCheckDeclOverride.cpp
bool checkOverrides(ValueDecl *decl);
void checkImplementationOnlyOverride(const ValueDecl *VD);

// Implemented in TypeCheckStorage.cpp
void setBoundVarsTypeError(Pattern *pattern, ASTContext &ctx);


/// How to generate the raw value for each element of an enum that doesn't
/// have one explicitly specified.
enum class AutomaticEnumValueKind {
  /// Raw values cannot be automatically generated.
  None,
  /// The raw value is the enum element's name.
  String,
  /// The raw value is the previous element's raw value, incremented.
  ///
  /// For the first element in the enum, the raw value is 0.
  Integer,
};

std::optional<AutomaticEnumValueKind>
computeAutomaticEnumValueKind(EnumDecl *ED);

void validatePrecedenceGroup(PrecedenceGroupDecl *PGD);

void diagnoseAttrsAddedByAccessNote(SourceFile &SF);

void checkVariadicParameters(ParameterList *params, DeclContext *dc);

}

#endif
