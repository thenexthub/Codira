//===- PackedVersion.cpp --------------------------------------------------===//
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
// Implements the Mach-O packed version.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TextAPI/PackedVersion.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/raw_ostream.h"

namespace vm::core {
namespace MachO {

bool PackedVersion::parse32(StringRef Str) {
  Version = 0;

  if (Str.empty())
    return false;

  SmallVector<StringRef, 3> Parts;
  SplitString(Str, Parts, ".");

  if (Parts.size() > 3 || Parts.empty())
    return false;

  unsigned long long Num;
  if (getAsUnsignedInteger(Parts[0], 10, Num))
    return false;

  if (Num > UINT16_MAX)
    return false;

  Version = Num << 16;

  for (unsigned i = 1, ShiftNum = 8; i < Parts.size(); ++i, ShiftNum -= 8) {
    if (getAsUnsignedInteger(Parts[i], 10, Num))
      return false;

    if (Num > UINT8_MAX)
      return false;

    Version |= (Num << ShiftNum);
  }

  return true;
}

std::pair<bool, bool> PackedVersion::parse64(StringRef Str) {
  bool Truncated = false;
  Version = 0;

  if (Str.empty())
    return std::make_pair(false, Truncated);

  SmallVector<StringRef, 5> Parts;
  SplitString(Str, Parts, ".");

  if (Parts.size() > 5 || Parts.empty())
    return std::make_pair(false, Truncated);

  unsigned long long Num;
  if (getAsUnsignedInteger(Parts[0], 10, Num))
    return std::make_pair(false, Truncated);

  if (Num > 0xFFFFFFULL)
    return std::make_pair(false, Truncated);

  if (Num > 0xFFFFULL) {
    Num = 0xFFFFULL;
    Truncated = true;
  }
  Version = Num << 16;

  for (unsigned i = 1, ShiftNum = 8; i < Parts.size() && i < 3;
       ++i, ShiftNum -= 8) {
    if (getAsUnsignedInteger(Parts[i], 10, Num))
      return std::make_pair(false, Truncated);

    if (Num > 0x3FFULL)
      return std::make_pair(false, Truncated);

    if (Num > 0xFFULL) {
      Num = 0xFFULL;
      Truncated = true;
    }
    Version |= (Num << ShiftNum);
  }

  if (Parts.size() > 3)
    Truncated = true;

  return std::make_pair(true, Truncated);
}

PackedVersion::operator std::string() const {
  SmallString<32> Str;
  raw_svector_ostream OS(Str);
  print(OS);
  return std::string(Str);
}

void PackedVersion::print(raw_ostream &OS) const {
  OS << format("%d", getMajor());
  if (getMinor() || getSubminor())
    OS << format(".%d", getMinor());
  if (getSubminor())
    OS << format(".%d", getSubminor());
}

} // end namespace MachO.
} // end namespace vm::core.
