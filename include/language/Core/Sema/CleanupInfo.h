//===--- CleanupInfo.cpp - Cleanup Control in Sema ------------------------===//
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
//  This file implements a set of operations on whether generating an
//  ExprWithCleanups in a full expression.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_CLEANUPINFO_H
#define LANGUAGE_CORE_SEMA_CLEANUPINFO_H

namespace language::Core {

class CleanupInfo {
  bool ExprNeedsCleanups = false;
  bool CleanupsHaveSideEffects = false;

public:
  bool exprNeedsCleanups() const { return ExprNeedsCleanups; }

  bool cleanupsHaveSideEffects() const { return CleanupsHaveSideEffects; }

  void setExprNeedsCleanups(bool SideEffects) {
    ExprNeedsCleanups = true;
    CleanupsHaveSideEffects |= SideEffects;
  }

  void reset() {
    ExprNeedsCleanups = false;
    CleanupsHaveSideEffects = false;
  }

  void mergeFrom(CleanupInfo Rhs) {
    ExprNeedsCleanups |= Rhs.ExprNeedsCleanups;
    CleanupsHaveSideEffects |= Rhs.CleanupsHaveSideEffects;
  }
};

} // end namespace language::Core

#endif
