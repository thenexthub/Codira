//===--- AttrDocTable.cpp - implements Attr::getDocumentation() -*- C++ -*-===//
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
//  This file contains out-of-line methods for Attr classes.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/Attr.h"
#include "toolchain/ADT/StringRef.h"

#include "AttrDocTable.inc"

static const toolchain::StringRef AttrDoc[] = {
#define ATTR(NAME) AttrDoc_##NAME,
#include "language/Core/Basic/AttrList.inc"
};

toolchain::StringRef language::Core::Attr::getDocumentation(language::Core::attr::Kind K) {
  if (K < (int)std::size(AttrDoc))
    return AttrDoc[K];
  return "";
}
