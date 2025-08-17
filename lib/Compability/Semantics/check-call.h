//===-- lib/Semantics/check-call.h ------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

// Constraint checking for procedure references

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_CHECK_CALL_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_CHECK_CALL_H_

#include "language/Compability/Evaluate/call.h"

namespace language::Compability::parser {
class Messages;
class ContextualMessages;
} // namespace language::Compability::parser
namespace language::Compability::evaluate::characteristics {
struct Procedure;
}
namespace language::Compability::evaluate {
class FoldingContext;
}

namespace language::Compability::semantics {
class Scope;
class SemanticsContext;

// Argument treatingExternalAsImplicit should be true when the called procedure
// does not actually have an explicit interface at the call site, but
// its characteristics are known because it is a subroutine or function
// defined at the top level in the same source file.  Returns false if
// messages were created, true if all is well.
bool CheckArguments(const evaluate::characteristics::Procedure &,
    evaluate::ActualArguments &, SemanticsContext &, const Scope &,
    bool treatingExternalAsImplicit, bool ignoreImplicitVsExplicit,
    const evaluate::SpecificIntrinsic *intrinsic);

bool CheckPPCIntrinsic(const Symbol &generic, const Symbol &specific,
    const evaluate::ActualArguments &actuals,
    evaluate::FoldingContext &context);
bool CheckWindowsIntrinsic(
    const Symbol &intrinsic, evaluate::FoldingContext &context);
bool CheckArgumentIsConstantExprInRange(
    const evaluate::ActualArguments &actuals, int index, int lowerBound,
    int upperBound, parser::ContextualMessages &messages);

// Checks actual arguments for the purpose of resolving a generic interface.
bool CheckInterfaceForGeneric(const evaluate::characteristics::Procedure &,
    evaluate::ActualArguments &, SemanticsContext &,
    bool allowActualArgumentConversions = false);
} // namespace language::Compability::semantics
#endif
