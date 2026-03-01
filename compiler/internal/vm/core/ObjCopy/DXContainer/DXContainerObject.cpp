//===- DXContainerObject.cpp ----------------------------------------------===//
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

#include "DXContainerObject.h"

namespace vm::core {
namespace objcopy {
namespace dxbc {

Error Object::removeParts(PartPred ToRemove) {
  erase_if(Parts, ToRemove);
  return Error::success();
}

void Object::recomputeHeader() {
  Header.FileSize = headerSize();
  Header.PartCount = Parts.size();
  for (const Part &P : Parts)
    Header.FileSize += P.size();
}

} // end namespace dxbc
} // end namespace objcopy
} // end namespace vm::core
