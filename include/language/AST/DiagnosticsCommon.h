//===--- DiagnosticsCommon.h - Shared Diagnostic Definitions ----*- C++ -*-===//
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
/// \file
/// This file defines common diagnostics for the whole compiler, as well
/// as some diagnostic infrastructure.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_DIAGNOSTICSCOMMON_H
#define SWIFT_DIAGNOSTICSCOMMON_H

#include "language/AST/DiagnosticEngine.h"
#include "language/Basic/LLVM.h"
#include "language/Config.h"

namespace language {
  class AccessorDecl;
  class ConstructorDecl;
  class MacroDecl;
  class SubscriptDecl;
  class SwitchStmt;
  class TypeAliasDecl;

  template<typename ...ArgTypes>
  struct Diag;

  namespace detail {
    // These templates are used to help extract the type arguments of the
    // DIAG/ERROR/WARNING/NOTE/REMARK/FIXIT macros.
    template<typename T>
    struct DiagWithArguments;
    
    template<typename ...ArgTypes>
    struct DiagWithArguments<void(ArgTypes...)> {
      typedef Diag<ArgTypes...> type;
    };

    template <typename T>
    struct StructuredFixItWithArguments;

    template <typename... ArgTypes>
    struct StructuredFixItWithArguments<void(ArgTypes...)> {
      typedef StructuredFixIt<ArgTypes...> type;
    };
  } // end namespace detail

  enum class StaticSpellingKind : uint8_t;

  namespace diag {

    enum class RequirementKind : uint8_t;

    using DeclAttribute = const DeclAttribute *;

  // Declare common diagnostics objects with their appropriate types.
#define DIAG(KIND, ID, Group, Options, Text, Signature)                    \
      extern detail::DiagWithArguments<void Signature>::type ID;
#define FIXIT(ID, Text, Signature)                                         \
      extern detail::StructuredFixItWithArguments<void Signature>::type ID;
#include "DiagnosticsCommon.def"
  } // end namespace diag
} // end namespace language

#endif
