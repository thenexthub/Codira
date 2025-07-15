//===--- DiagnosticList.cpp - Diagnostic Definitions ----------------------===//
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
//  This file defines all of the diagnostics emitted by Codira.
//
//===----------------------------------------------------------------------===//

#include "language/AST/DiagnosticList.h"
#include "language/AST/DiagnosticsCommon.h"
#include "language/Basic/Assertions.h"
using namespace language;

// Define all of the diagnostic objects and initialize them with their 
// diagnostic IDs.
namespace language {
  namespace diag {
#define DIAG(KIND, ID, Group, Options, Text, Signature)                      \
    detail::DiagWithArguments<void Signature>::type ID = {DiagID::ID};
#define FIXIT(ID, Text, Signature) \
    detail::StructuredFixItWithArguments<void Signature>::type ID = {FixItID::ID};
#include "language/AST/DiagnosticsAll.def"
  } // end namespace diag
} // end namespace language
