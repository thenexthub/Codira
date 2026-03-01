//===---------- ObjectFormats.cpp - Object format details for ORC ---------===//
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
// ORC-specific object format details.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/Orc/Shared/ObjectFormats.h"
#include "vm/core/ADT/STLExtras.h"

namespace vm::core {
namespace orc {

StringRef ELFEHFrameSectionName = ".eh_frame";

StringRef ELFInitArrayFuncSectionName = ".init_array";
StringRef ELFInitFuncSectionName = ".init";
StringRef ELFFiniArrayFuncSectionName = ".fini_array";
StringRef ELFFiniFuncSectionName = ".fini";
StringRef ELFCtorArrayFuncSectionName = ".ctors";
StringRef ELFDtorArrayFuncSectionName = ".dtors";

StringRef ELFInitSectionNames[3]{
    ELFInitArrayFuncSectionName,
    ELFInitFuncSectionName,
    ELFCtorArrayFuncSectionName,
};

StringRef ELFThreadBSSSectionName = ".tbss";
StringRef ELFThreadDataSectionName = ".tdata";

bool isMachOInitializerSection(StringRef QualifiedName) {
  return toolchain::is_contained(MachOInitSectionNames, QualifiedName);
}

bool isELFInitializerSection(StringRef SecName) {
  for (StringRef InitSection : ELFInitSectionNames) {
    StringRef Name = SecName;
    if (Name.consume_front(InitSection) && (Name.empty() || Name[0] == '.'))
      return true;
  }
  return false;
}

bool isCOFFInitializerSection(StringRef SecName) {
  return SecName.starts_with(".CRT");
}

} // namespace orc
} // namespace vm::core
