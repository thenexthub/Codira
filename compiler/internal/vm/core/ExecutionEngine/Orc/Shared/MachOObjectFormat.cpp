//===-------- MachOObjectFormat.cpp -- MachO format details for ORC -------===//
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
// ORC-specific MachO object format details.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ExecutionEngine/Orc/Shared/MachOObjectFormat.h"

namespace vm::core {
namespace orc {

StringRef MachODataCommonSectionName = "__DATA,__common";
StringRef MachODataDataSectionName = "__DATA,__data";
StringRef MachOEHFrameSectionName = "__TEXT,__eh_frame";
StringRef MachOCompactUnwindSectionName = "__LD,__compact_unwind";
StringRef MachOCStringSectionName = "__TEXT,__cstring";
StringRef MachOModInitFuncSectionName = "__DATA,__mod_init_func";
StringRef MachOObjCCatListSectionName = "__DATA,__objc_catlist";
StringRef MachOObjCCatList2SectionName = "__DATA,__objc_catlist2";
StringRef MachOObjCClassListSectionName = "__DATA,__objc_classlist";
StringRef MachOObjCClassNameSectionName = "__TEXT,__objc_classname";
StringRef MachOObjCClassRefsSectionName = "__DATA,__objc_classrefs";
StringRef MachOObjCConstSectionName = "__DATA,__objc_const";
StringRef MachOObjCDataSectionName = "__DATA,__objc_data";
StringRef MachOObjCImageInfoSectionName = "__DATA,__objc_imageinfo";
StringRef MachOObjCMethNameSectionName = "__TEXT,__objc_methname";
StringRef MachOObjCMethTypeSectionName = "__TEXT,__objc_methtype";
StringRef MachOObjCNLCatListSectionName = "__DATA,__objc_nlcatlist";
StringRef MachOObjCNLClassListSectionName = "__DATA,__objc_nlclslist";
StringRef MachOObjCProtoListSectionName = "__DATA,__objc_protolist";
StringRef MachOObjCProtoRefsSectionName = "__DATA,__objc_protorefs";
StringRef MachOObjCSelRefsSectionName = "__DATA,__objc_selrefs";
StringRef MachOSwift5ProtoSectionName = "__TEXT,__swift5_proto";
StringRef MachOSwift5ProtosSectionName = "__TEXT,__swift5_protos";
StringRef MachOSwift5TypesSectionName = "__TEXT,__swift5_types";
StringRef MachOSwift5TypeRefSectionName = "__TEXT,__swift5_typeref";
StringRef MachOSwift5FieldMetadataSectionName = "__TEXT,__swift5_fieldmd";
StringRef MachOSwift5EntrySectionName = "__TEXT,__swift5_entry";
StringRef MachOTextTextSectionName = "__TEXT,__text";
StringRef MachOThreadBSSSectionName = "__DATA,__thread_bss";
StringRef MachOThreadDataSectionName = "__DATA,__thread_data";
StringRef MachOThreadVarsSectionName = "__DATA,__thread_vars";
StringRef MachOUnwindInfoSectionName = "__TEXT,__unwind_info";

StringRef MachOInitSectionNames[22] = {
    MachOModInitFuncSectionName,         MachOObjCCatListSectionName,
    MachOObjCCatList2SectionName,        MachOObjCClassListSectionName,
    MachOObjCClassNameSectionName,       MachOObjCClassRefsSectionName,
    MachOObjCConstSectionName,           MachOObjCDataSectionName,
    MachOObjCImageInfoSectionName,       MachOObjCMethNameSectionName,
    MachOObjCMethTypeSectionName,        MachOObjCNLCatListSectionName,
    MachOObjCNLClassListSectionName,     MachOObjCProtoListSectionName,
    MachOObjCProtoRefsSectionName,       MachOObjCSelRefsSectionName,
    MachOSwift5ProtoSectionName,         MachOSwift5ProtosSectionName,
    MachOSwift5TypesSectionName,         MachOSwift5TypeRefSectionName,
    MachOSwift5FieldMetadataSectionName, MachOSwift5EntrySectionName,
};

bool isMachOInitializerSection(StringRef SegName, StringRef SecName) {
  for (auto &InitSection : MachOInitSectionNames) {
    // Loop below assumes all MachO init sectios have a length-6
    // segment name.
    assert(InitSection[6] == ',' && "Init section seg name has length != 6");
    if (InitSection.starts_with(SegName) && InitSection.substr(7) == SecName)
      return true;
  }
  return false;
}

} // namespace orc
} // namespace vm::core
