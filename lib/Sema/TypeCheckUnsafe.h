//===--- TypeCheckUnasfe.h - Strict Safety Diagnostics ----------*- C++ -*-===//
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

#ifndef SWIFT_SEMA_TYPE_CHECK_UNSAFE_H
#define SWIFT_SEMA_TYPE_CHECK_UNSAFE_H

#include "language/AST/UnsafeUse.h"

namespace llvm {
template <typename Fn> class function_ref;
}

namespace language {

class Witness;

/// Diagnose the given unsafe use right now.
void diagnoseUnsafeUse(const UnsafeUse &use);

/// Enumerate all of the unsafe uses that occur within this declaration
///
/// The given `fn` will be called with each unsafe use. If it returns `true`
/// for any use, this function will return `true` immediately. Otherwise,
/// it will return `false` once all unsafe uses have been emitted.
bool enumerateUnsafeUses(ConcreteDeclRef declRef,
                         SourceLoc loc,
                         bool isCall,
                         llvm::function_ref<bool(UnsafeUse)> fn);

/// Enumerate all of the unsafe uses that occur within this array of protocol
/// conformances.
///
/// The given `fn` will be called with each unsafe use. If it returns `true`
/// for any use, this function will return `true` immediately. Otherwise,
/// it will return `false` once all unsafe uses have been emitted.
bool enumerateUnsafeUses(ArrayRef<ProtocolConformanceRef> conformances,
                         SourceLoc loc,
                         llvm::function_ref<bool(UnsafeUse)> fn);

/// Enumerate all of the unsafe uses that occur within this substitution map.
///
/// The given `fn` will be called with each unsafe use. If it returns `true`
/// for any use, this function will return `true` immediately. Otherwise,
/// it will return `false` once all unsafe uses have been emitted.
bool enumerateUnsafeUses(SubstitutionMap subs,
                         SourceLoc loc,
                         llvm::function_ref<bool(UnsafeUse)> fn);

/// Determine whether a reference to this declaration is considered unsafe,
/// either explicitly (@unsafe) or because it references an unsafe type.
bool isUnsafe(ConcreteDeclRef declRef);

/// Whether the given requirement should be considered unsafe for the given
/// conformance.
bool isUnsafeInConformance(const ValueDecl *requirement,
                           const Witness &witness,
                           NormalProtocolConformance *conformance);

/// If the given type involves an unsafe type, diagnose it by calling the
/// diagnose function with the most specific unsafe type that can be provided.
void diagnoseUnsafeType(ASTContext &ctx, SourceLoc loc, Type type,
                        llvm::function_ref<void(Type)> diagnose);

/// Check for unsafe storage within this nominal type declaration.
void checkUnsafeStorage(NominalTypeDecl *nominal);

}

#endif // SWIFT_SEMA_TYPE_CHECK_UNSAFE_H
