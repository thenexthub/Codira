//===- ArchitectureSet.cpp ------------------------------------------------===//
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
// Implements the architecture set.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TextAPI/ArchitectureSet.h"
#include "vm/core/Support/raw_ostream.h"

namespace vm::core {
namespace MachO {

ArchitectureSet::ArchitectureSet(const std::vector<Architecture> &Archs)
    : ArchitectureSet() {
  for (auto Arch : Archs) {
    if (Arch == AK_unknown)
      continue;
    set(Arch);
  }
}

size_t ArchitectureSet::count() const {
  // popcnt
  size_t Cnt = 0;
  for (unsigned i = 0; i < sizeof(ArchSetType) * 8; ++i)
    if (ArchSet & (1U << i))
      ++Cnt;
  return Cnt;
}

ArchitectureSet::operator std::string() const {
  if (empty())
    return "[(empty)]";

  std::string result;
  auto size = count();
  for (auto arch : *this) {
    result.append(std::string(getArchitectureName(arch)));
    size -= 1;
    if (size)
      result.append(" ");
  }
  return result;
}

ArchitectureSet::operator std::vector<Architecture>() const {
  std::vector<Architecture> archs;
  for (auto arch : *this) {
    if (arch == AK_unknown)
      continue;
    archs.emplace_back(arch);
  }
  return archs;
}

void ArchitectureSet::print(raw_ostream &os) const { os << std::string(*this); }

raw_ostream &operator<<(raw_ostream &os, ArchitectureSet set) {
  set.print(os);
  return os;
}

} // end namespace MachO.
} // end namespace vm::core.
