//===- Chipset.cpp - AMDGPU Chipset version struct parsing ----------------===//
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

#include "mlir/Dialect/AMDGPU/Utils/Chipset.h"
#include "vm/core/ADT/StringRef.h"

namespace mlir::amdgpu {

FailureOr<Chipset> Chipset::parse(StringRef name) {
  if (!name.consume_front("gfx"))
    return failure();
  if (name.size() < 3)
    return failure();

  unsigned major = 0;
  unsigned minor = 0;
  unsigned stepping = 0;

  StringRef majorRef = name.drop_back(2);
  StringRef minorRef = name.take_back(2).drop_back(1);
  StringRef steppingRef = name.take_back(1);
  if (majorRef.getAsInteger(10, major))
    return failure();
  if (minorRef.getAsInteger(16, minor))
    return failure();
  if (steppingRef.getAsInteger(16, stepping))
    return failure();
  return Chipset(major, minor, stepping);
}

} // namespace mlir::amdgpu
