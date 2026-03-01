//===- DXContainerReader.cpp ----------------------------------------------===//
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

#include "DXContainerReader.h"

namespace vm::core {
namespace objcopy {
namespace dxbc {

using namespace object;

Expected<std::unique_ptr<Object>> DXContainerReader::create() const {
  auto Obj = std::make_unique<Object>();
  Obj->Header = DXContainerObj.getHeader();
  for (const SectionRef &Part : DXContainerObj.sections()) {
    DataRefImpl PartDRI = Part.getRawDataRefImpl();
    Expected<StringRef> Name = DXContainerObj.getSectionName(PartDRI);
    if (auto E = Name.takeError())
      return E;
    assert(Name->size() == 4 &&
           "Valid DXIL Part name consists of 4 characters");
    Expected<ArrayRef<uint8_t>> Data =
        DXContainerObj.getSectionContents(PartDRI);
    if (auto E = Data.takeError())
      return E;
    Obj->Parts.push_back({*Name, *Data});
  }
  return std::move(Obj);
}

} // end namespace dxbc
} // end namespace objcopy
} // end namespace vm::core
