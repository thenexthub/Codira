//===- CocoaConventions.h - Special handling of Cocoa conventions -*- C++ -*--//
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
// This file implements cocoa naming convention analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_DOMAINSPECIFIC_COCOACONVENTIONS_H
#define LANGUAGE_CORE_ANALYSIS_DOMAINSPECIFIC_COCOACONVENTIONS_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
class FunctionDecl;
class QualType;

namespace ento {
namespace cocoa {

  bool isRefType(QualType RetTy, StringRef Prefix,
                 StringRef Name = StringRef());

  bool isCocoaObjectRef(QualType T);

}

namespace coreFoundation {
  bool isCFObjectRef(QualType T);

  bool followsCreateRule(const FunctionDecl *FD);
}

}} // end: "clang:ento"

#endif
