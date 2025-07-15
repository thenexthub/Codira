//===-- Sema/NonisolatedNonsendingByDefaultMigration.h ----------*- C++ -*-===//
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
///
/// \file
/// This file provides code migration support for the
/// `NonisolatedNonsendingByDefault` feature.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SEMA_NONISOLATEDNONSENDINGBYDEFAULTMIGRATION_H
#define LANGUAGE_SEMA_NONISOLATEDNONSENDINGBYDEFAULTMIGRATION_H

#include "language/AST/ActorIsolation.h"
#include "language/AST/ExtInfo.h"

namespace language {

class FunctionTypeRepr;
class ValueDecl;
class AbstractClosureExpr;

/// Warns that the behavior of nonisolated async functions will change under
/// `NonisolatedNonsendingByDefault` and suggests `@concurrent` to preserve the current
/// behavior.
void warnAboutNewNonisolatedAsyncExecutionBehavior(
    ASTContext &ctx, FunctionTypeRepr *node, FunctionTypeIsolation isolation);

/// Warns that the behavior of nonisolated async functions will change under
/// `NonisolatedNonsendingByDefault` and suggests `@concurrent` to preserve the current
/// behavior.
void warnAboutNewNonisolatedAsyncExecutionBehavior(ASTContext &ctx,
                                                   ValueDecl *node,
                                                   ActorIsolation isolation);

/// Warns that the behavior of nonisolated async functions will change under
/// `NonisolatedNonsendingByDefault` and suggests `@concurrent` to preserve the current
/// behavior.
void warnAboutNewNonisolatedAsyncExecutionBehavior(ASTContext &ctx,
                                                   AbstractClosureExpr *node,
                                                   ActorIsolation isolation);

} // end namespace language

#endif /* LANGUAGE_SEMA_NONISOLATEDNONSENDINGBYDEFAULTMIGRATION_H */
