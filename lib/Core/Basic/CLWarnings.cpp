//===--- CLWarnings.h - Maps some cl.exe warning ids  -----------*- C++ -*-===//
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
//
//  This file implements the Diagnostic-related interfaces.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/CLWarnings.h"
#include "language/Core/Basic/DiagnosticCategories.h"
#include <optional>

using namespace language::Core;

std::optional<diag::Group>
language::Core::diagGroupFromCLWarningID(unsigned CLWarningID) {
  switch (CLWarningID) {
  case 4005: return diag::Group::MacroRedefined;
  case 4018: return diag::Group::SignCompare;
  case 4100: return diag::Group::UnusedParameter;
  case 4910: return diag::Group::DllexportExplicitInstantiationDecl;
  case 4996: return diag::Group::DeprecatedDeclarations;
  }
  return {};
}
