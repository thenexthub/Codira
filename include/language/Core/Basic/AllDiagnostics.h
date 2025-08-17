//===--- AllDiagnostics.h - Aggregate Diagnostic headers --------*- C++ -*-===//
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
///
/// \file
/// Includes all the separate Diagnostic headers & some related helpers.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_ALLDIAGNOSTICS_H
#define LANGUAGE_CORE_BASIC_ALLDIAGNOSTICS_H

#include "language/Core/Basic/DiagnosticAST.h"
#include "language/Core/Basic/DiagnosticAnalysis.h"
#include "language/Core/Basic/DiagnosticComment.h"
#include "language/Core/Basic/DiagnosticCrossTU.h"
#include "language/Core/Basic/DiagnosticDriver.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "language/Core/Basic/DiagnosticInstallAPI.h"
#include "language/Core/Basic/DiagnosticLex.h"
#include "language/Core/Basic/DiagnosticParse.h"
#include "language/Core/Basic/DiagnosticSema.h"
#include "language/Core/Basic/DiagnosticSerialization.h"
#include "language/Core/Basic/DiagnosticRefactoring.h"

namespace language::Core {
template <size_t SizeOfStr, typename FieldType>
class StringSizerHelper {
  static_assert(SizeOfStr <= FieldType(~0U), "Field too small!");
public:
  enum { Size = SizeOfStr };
};
} // end namespace language::Core

#define STR_SIZE(str, fieldTy) language::Core::StringSizerHelper<sizeof(str)-1, \
                                                        fieldTy>::Size

#endif
