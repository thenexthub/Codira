//===--- TypeCheckAccess.h - Type Checking for Access Control --*- C++ -*-===//
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
// This file implements access control checking.
//
//===----------------------------------------------------------------------===//

#ifndef TYPECHECKACCESS_H
#define TYPECHECKACCESS_H

#include <cstdint>

namespace language {

class Decl;
class ExportContext;
class SourceFile;

/// Performs access-related checks for \p D.
///
/// At a high level, this checks the given declaration's signature does not
/// reference any other declarations that are less visible than the declaration
/// itself. Related checks may also be performed.
void checkAccessControl(Decl *D);

/// Problematic origin of an exported type.
///
/// This enum must be kept in sync with a number of diagnostics:
///   diag::inlinable_decl_ref_from_hidden_module
///   diag::decl_from_hidden_module
///   diag::conformance_from_implementation_only_module
///   diag::typealias_desugars_to_type_from_hidden_module
///   daig::inlinable_typealias_desugars_to_type_from_hidden_module
enum class DisallowedOriginKind : uint8_t {
  ImplementationOnly,
  SPIImported,
  SPILocal,
  SPIOnly,
  MissingImport,
  FragileCxxAPI,
  NonPublicImport,
  None
};

/// A uniquely-typed boolean to reduce the chances of accidentally inverting
/// a check.
///
/// \see checkTypeAccess
enum class DowngradeToWarning: bool {
  No,
  Yes
};

/// Returns the kind of origin, implementation-only import or SPI declaration,
/// that restricts exporting \p decl from the given file and context.
DisallowedOriginKind getDisallowedOriginKind(const Decl *decl,
                                             const ExportContext &where);

DisallowedOriginKind getDisallowedOriginKind(const Decl *decl,
                                             const ExportContext &where,
                                             DowngradeToWarning &downgradeToWarning);

} // end namespace language

#endif
