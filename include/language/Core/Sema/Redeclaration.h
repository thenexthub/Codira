//===- Redeclaration.h - Redeclarations--------------------------*- C++ -*-===//
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
//  This file defines RedeclarationKind enum.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_REDECLARATION_H
#define LANGUAGE_CORE_SEMA_REDECLARATION_H

/// Specifies whether (or how) name lookup is being performed for a
/// redeclaration (vs. a reference).
enum class RedeclarationKind {
  /// The lookup is a reference to this name that is not for the
  /// purpose of redeclaring the name.
  NotForRedeclaration = 0,
  /// The lookup results will be used for redeclaration of a name,
  /// if an entity by that name already exists and is visible.
  ForVisibleRedeclaration,
  /// The lookup results will be used for redeclaration of a name
  /// with external linkage; non-visible lookup results with external linkage
  /// may also be found.
  ForExternalRedeclaration
};

#endif // LANGUAGE_CORE_SEMA_REDECLARATION_H
