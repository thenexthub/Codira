//===- Sections.cpp ---------------------------------------------------===//
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

#include "Sections.h"
#include "InputSection.h"
#include "OutputSegment.h"

#include "llvm/ADT/StringSwitch.h"

using namespace llvm;
using namespace llvm::MachO;

namespace lld::macho::sections {
bool isCodeSection(StringRef name, StringRef segName, uint32_t flags) {
  uint32_t type = sectionType(flags);
  if (type != S_REGULAR && type != S_COALESCED)
    return false;

  uint32_t attr = flags & SECTION_ATTRIBUTES_USR;
  if (attr == S_ATTR_PURE_INSTRUCTIONS)
    return true;

  if (segName == segment_names::text)
    return StringSwitch<bool>(name)
        .Cases({section_names::textCoalNt, section_names::staticInit}, true)
        .Default(false);

  return false;
}

} // namespace lld::macho::sections
