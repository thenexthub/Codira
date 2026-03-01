//===- DXContainerWriter.cpp ----------------------------------------------===//
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

#include "DXContainerWriter.h"

namespace vm::core {
namespace objcopy {
namespace dxbc {

using namespace object;

size_t DXContainerWriter::finalize() {
  assert(Offsets.empty() &&
         "Attempted to finalize writer with already computed offsets");
  Offsets.reserve(Obj.Parts.size());
  size_t Offset = Obj.headerSize();
  for (const Part &P : Obj.Parts) {
    Offsets.push_back(Offset);
    Offset += P.size();
  }
  return Obj.Header.FileSize;
}

Error DXContainerWriter::write() {
  size_t TotalSize = finalize();
  Out.reserveExtraSpace(TotalSize);

  toolchain::dxbc::Header Header = Obj.Header;
  if (sys::IsBigEndianHost)
    Header.swapBytes();
  Out.write(reinterpret_cast<const char *>(&Header),
            sizeof(::toolchain::dxbc::Header));
  if (sys::IsBigEndianHost)
    for (auto &O : Offsets)
      sys::swapByteOrder(O);
  Out.write(reinterpret_cast<const char *>(Offsets.data()),
            Offsets.size() * sizeof(uint32_t));

  for (const Part &P : Obj.Parts) {
    Out.write(reinterpret_cast<const char *>(P.Name.data()), 4);
    uint32_t Size = P.Data.size();
    if (sys::IsBigEndianHost)
      sys::swapByteOrder(Size);
    Out.write(reinterpret_cast<const char *>(&Size), sizeof(uint32_t));
    Out.write(reinterpret_cast<const char *>(P.Data.data()), P.Data.size());
  }

  return Error::success();
}

} // end namespace dxbc
} // end namespace objcopy
} // end namespace vm::core
