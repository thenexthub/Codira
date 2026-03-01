//===-- TestSectionFileSize.cpp -------------------------------------------===//
//
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

#include "gtest/gtest.h"

#include "Plugins/ObjectFile/PECOFF/ObjectFilePECOFF.h"
#include "Plugins/Process/Utility/lldb-x86-register-enums.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/TestUtilities.h"

#include "lldb/Core/Module.h"
#include "lldb/Symbol/CallFrameInfo.h"
#include "lldb/Symbol/UnwindPlan.h"
#include "llvm/Testing/Support/Error.h"

using namespace lldb_private;
using namespace lldb;

class SectionSizeTest : public testing::Test {
  SubsystemRAII<FileSystem, ObjectFilePECOFF> subsystems;
};

TEST_F(SectionSizeTest, NoAlignmentPadding) {
  llvm::Expected<TestFile> ExpectedFile = TestFile::fromYaml(
      R"(
--- !COFF
OptionalHeader:
  SectionAlignment: 4096
  FileAlignment:   512
header:
  Machine:         IMAGE_FILE_MACHINE_AMD64
  Characteristics: [ IMAGE_FILE_EXECUTABLE_IMAGE, IMAGE_FILE_LARGE_ADDRESS_AWARE ]
sections:
  - Name:            swiftast
    VirtualSize:     496
    SizeOfRawData:   512
    Characteristics: [ IMAGE_SCN_CNT_INITIALIZED_DATA, IMAGE_SCN_MEM_READ ]
    SectionData:     11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111110000

symbols:         []
...
)");
  ASSERT_THAT_EXPECTED(ExpectedFile, llvm::Succeeded());

  ModuleSP module_sp = std::make_shared<Module>(ExpectedFile->moduleSpec());
  ObjectFile *object_file = module_sp->GetObjectFile();
  ASSERT_NE(object_file, nullptr);

  SectionList *section_list = object_file->GetSectionList();
  ASSERT_NE(section_list, nullptr);

  SectionSP swiftast_section;
  size_t section_count = section_list->GetNumSections(0);
  for (size_t i = 0; i < section_count; ++i) {
    SectionSP section_sp = section_list->GetSectionAtIndex(i);
    if (section_sp->GetName() == "swiftast") {
      swiftast_section = section_sp;
      break;
    }
  }
  ASSERT_NE(swiftast_section.get(), nullptr);

  DataExtractor section_data;
  ASSERT_NE(object_file->ReadSectionData(swiftast_section.get(),
                                         section_data),
            (size_t)0);

  // Check that the section data size is equal to VirtualSize (496)
  // without the zero padding, instead of SizeOfRawData (512).
  EXPECT_EQ(section_data.GetByteSize(), (uint64_t)496);
}

