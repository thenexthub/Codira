//===--- CFProtectionOptions.h ----------------------------------*- C++ -*-===//
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
//  This file defines constants for -fcf-protection and other related flags.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_CFPROTECTIONOPTIONS_H
#define LANGUAGE_CORE_BASIC_CFPROTECTIONOPTIONS_H

#include "toolchain/Support/ErrorHandling.h"

namespace language::Core {

enum class CFBranchLabelSchemeKind {
  Default,
#define CF_BRANCH_LABEL_SCHEME(Kind, FlagVal) Kind,
#include "language/Core/Basic/CFProtectionOptions.def"
};

static inline const char *
getCFBranchLabelSchemeFlagVal(const CFBranchLabelSchemeKind Scheme) {
#define CF_BRANCH_LABEL_SCHEME(Kind, FlagVal)                                  \
  if (Scheme == CFBranchLabelSchemeKind::Kind)                                 \
    return #FlagVal;
#include "language/Core/Basic/CFProtectionOptions.def"

  toolchain::report_fatal_error("invalid scheme");
}

} // namespace language::Core

#endif // #ifndef LANGUAGE_CORE_BASIC_CFPROTECTIONOPTIONS_H
