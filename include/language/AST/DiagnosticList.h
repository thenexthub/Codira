//===--- DiagnosticList.h - Diagnostic Definitions --------------*- C++ -*-===//
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
//  This file defines all of the diagnostics emitted by Swift.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_DIAGNOSTICLIST_H
#define SWIFT_DIAGNOSTICLIST_H

#include <cstdint>
#include <type_traits>

namespace language {

/// Enumeration describing all of possible diagnostics.
///
/// Each of the diagnostics described in Diagnostics.def has an entry in
/// this enumeration type that uniquely identifies it.
enum class DiagID : uint32_t {
#define DIAG(KIND, ID, Group, Options, Text, Signature) ID,
#include "language/AST/DiagnosticsAll.def"
  NumDiagsHandle
};
static_assert(static_cast<uint32_t>(swift::DiagID::invalid_diagnostic) == 0,
              "0 is not the invalid diagnostic ID");

constexpr auto NumDiagIDs =
    static_cast<std::underlying_type_t<DiagID>>(DiagID::NumDiagsHandle);

enum class FixItID : uint32_t {
#define DIAG(KIND, ID, Group, Options, Text, Signature)
#define FIXIT(ID, Text, Signature) ID,
#include "language/AST/DiagnosticsAll.def"
};

} // end namespace language

#endif /* SWIFT_DIAGNOSTICLIST_H */
