//===- DiagnosticCategories.h - Diagnostic Categories Enumerators-*- C++ -*===//
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

#ifndef LANGUAGE_CORE_BASIC_DIAGNOSTICCATEGORIES_H
#define LANGUAGE_CORE_BASIC_DIAGNOSTICCATEGORIES_H

namespace language::Core {
  namespace diag {
    enum DiagCategory {
#define GET_CATEGORY_TABLE
#define CATEGORY(X, ENUM) ENUM,
#include "language/Core/Basic/DiagnosticGroups.inc"
#undef CATEGORY
#undef GET_CATEGORY_TABLE
      DiagCat_NUM_CATEGORIES
    };

    enum class Group {
#define DIAG_ENTRY(GroupName, FlagNameOffset, Members, SubGroups, Docs)    \
      GroupName,
#include "language/Core/Basic/DiagnosticGroups.inc"
#undef CATEGORY
#undef DIAG_ENTRY
      NUM_GROUPS
    };
  }  // end namespace diag
}  // end namespace language::Core

#endif
