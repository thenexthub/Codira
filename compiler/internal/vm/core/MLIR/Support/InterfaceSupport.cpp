//===- InterfaceSupport.cpp - MLIR Interface Support Classes --------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file defines several support classes for defining interfaces.
//
//===----------------------------------------------------------------------===//

#include "mlir/Support/InterfaceSupport.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

#define DEBUG_TYPE "interfaces"

using namespace mlir;

void detail::InterfaceMap::insert(TypeID interfaceId, void *conceptImpl) {
  // Insert directly into the right position to keep the interfaces sorted.
  auto *it =
      toolchain::lower_bound(interfaces, interfaceId, [](const auto &it, TypeID id) {
        return compare(it.first, id);
      });
  if (it != interfaces.end() && it->first == interfaceId) {
    LLVM_DEBUG(toolchain::dbgs() << "Ignoring repeated interface registration\n");
    free(conceptImpl);
    return;
  }
  interfaces.insert(it, {interfaceId, conceptImpl});
}
