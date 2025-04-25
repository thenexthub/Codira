//===-- TypeCheckDistributed.h - Distributed actor typechecking -*- C++ -*-===//
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
// This file provides type checking support for Swift's distributed actor model.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SEMA_TYPECHECKDISTRIBUTED_H
#define SWIFT_SEMA_TYPECHECKDISTRIBUTED_H

#include "language/AST/ConcreteDeclRef.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/Type.h"

namespace language {

class ClassDecl;
class ConstructorDecl;
class Decl;
class DeclContext;
class FuncDecl;
class NominalTypeDecl;

/******************************************************************************/
/********************* Distributed Actor Type Checking ************************/
/******************************************************************************/

// Diagnose an error if the Distributed module is not loaded.
bool ensureDistributedModuleLoaded(const ValueDecl *decl);

/// Check for illegal property declarations (e.g. re-declaring transport or id)
void checkDistributedActorProperties(const NominalTypeDecl *decl);

/// Type-check additional ad-hoc protocol requirements.
/// Ad-hoc requirements are protocol requirements currently not expressible
/// in the Swift type-system.
bool checkDistributedActorSystemAdHocProtocolRequirements(
    ASTContext &Context,
    ProtocolDecl *Proto,
    NormalProtocolConformance *Conformance,
    Type Adoptee,
    bool diagnose);

/// Check 'DistributedActorSystem' implementations for additional restrictions.
bool checkDistributedActorSystem(const NominalTypeDecl *system);

/// Typecheck a distributed method declaration
bool checkDistributedFunction(AbstractFunctionDecl *decl);

/// Typecheck a distributed computed (get-only) property declaration.
/// They are effectively checked the same way as argument-less methods.
bool checkDistributedActorProperty(VarDecl *decl, bool diagnose);

/// Diagnose a distributed func declaration in a not-distributed actor protocol.
void diagnoseDistributedFunctionInNonDistributedActorProtocol(
  const ProtocolDecl *proto, InFlightDiagnostic &diag);

/// Emit a FixIt suggesting to add Codable to the nominal type.
void addCodableFixIt(const NominalTypeDecl *nominal, InFlightDiagnostic &diag);

}

#endif /* SWIFT_SEMA_TYPECHECKDISTRIBUTED_H */
