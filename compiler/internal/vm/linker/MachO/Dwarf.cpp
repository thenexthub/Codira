//===- DWARF.cpp ----------------------------------------------------------===//
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

#include "Dwarf.h"
#include "InputFiles.h"
#include "InputSection.h"
#include "OutputSegment.h"

#include <memory>

using namespace lld;
using namespace lld::macho;
using namespace llvm;

std::unique_ptr<DwarfObject> DwarfObject::create(ObjFile *obj) {
  auto dObj = std::make_unique<DwarfObject>();
  bool hasDwarfInfo = false;
  // LLD only needs to extract the source file path and line numbers from the
  // debug info, so we initialize DwarfObject with just the sections necessary
  // to get that path. The debugger will locate the debug info via the object
  // file paths that we emit in our STABS symbols, so we don't need to process &
  // emit them ourselves.
  for (const InputSection *isec : obj->debugSections) {
    if (StringRef *s =
            StringSwitch<StringRef *>(isec->getName())
                .Case(section_names::debugInfo, &dObj->infoSection.Data)
                .Case(section_names::debugLine, &dObj->lineSection.Data)
                .Case(section_names::debugStrOffs, &dObj->strOffsSection.Data)
                .Case(section_names::debugAbbrev, &dObj->abbrevSection)
                .Case(section_names::debugStr, &dObj->strSection)
                .Default(nullptr)) {
      *s = toStringRef(isec->data);
      hasDwarfInfo = true;
    }
  }

  if (hasDwarfInfo)
    return dObj;
  return nullptr;
}
