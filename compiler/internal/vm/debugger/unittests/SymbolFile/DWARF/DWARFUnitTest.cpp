//===-- DWARFUnitTest.cpp -------------------------------------------------===//
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

#include "Plugins/SymbolFile/DWARF/DWARFUnit.h"
#include "TestingSupport/Symbol/YAMLModuleTester.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

TEST(DWARFUnitTest, NullUnitDie) {
  // Make sure we don't crash parsing a null unit DIE.
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_language
              Form:            DW_FORM_data2
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x00000000
)";

  YAMLModuleTester t(yamldata);
  ASSERT_TRUE((bool)t.GetDwarfUnit());

  DWARFUnit *unit = t.GetDwarfUnit();
  const DWARFDebugInfoEntry *die_first = unit->DIE().GetDIE();
  ASSERT_NE(die_first, nullptr);
  EXPECT_TRUE(die_first->IsNULL());
}

TEST(DWARFUnitTest, MissingSentinel) {
  // Make sure we don't crash if the debug info is missing a null DIE sentinel.
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_language
              Form:            DW_FORM_data2
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x00000001
          Values:
            - Value:           0x000000000000000C
)";

  YAMLModuleTester t(yamldata);
  ASSERT_TRUE((bool)t.GetDwarfUnit());

  DWARFUnit *unit = t.GetDwarfUnit();
  const DWARFDebugInfoEntry *die_first = unit->DIE().GetDIE();
  ASSERT_NE(die_first, nullptr);
  EXPECT_EQ(die_first->GetFirstChild(), nullptr);
  EXPECT_EQ(die_first->GetSibling(), nullptr);
}

TEST(DWARFUnitTest, ClangProducer) {
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_str:
    - 'Apple clang version 13.0.0 (clang-1300.0.29.3)'
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_producer
              Form:            DW_FORM_strp
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x1
          Values:
            - Value:           0x0
        - AbbrCode:        0x0
)";

  YAMLModuleTester t(yamldata);
  DWARFUnit *unit = t.GetDwarfUnit();
  ASSERT_TRUE((bool)unit);
  EXPECT_EQ(unit->GetProducer(), eProducerClang);
  EXPECT_EQ(unit->GetProducerVersion(), llvm::VersionTuple(1300, 0, 29, 3));
}

TEST(DWARFUnitTest, SwiftProducer) {
  const char *yamldata = R"(
--- !ELF
FileHeader:
  Class:   ELFCLASS64
  Data:    ELFDATA2LSB
  Type:    ET_EXEC
  Machine: EM_386
DWARF:
  debug_str:
    - 'Apple Swift version 5.5 (swiftlang-1300.0.31.1 clang-1300.0.29.1)'
  debug_abbrev:
    - Table:
        - Code:            0x00000001
          Tag:             DW_TAG_compile_unit
          Children:        DW_CHILDREN_yes
          Attributes:
            - Attribute:       DW_AT_producer
              Form:            DW_FORM_strp
  debug_info:
    - Version:         4
      AddrSize:        8
      Entries:
        - AbbrCode:        0x1
          Values:
            - Value:           0x0
        - AbbrCode:        0x0
)";

  YAMLModuleTester t(yamldata);
  DWARFUnit *unit = t.GetDwarfUnit();
  ASSERT_TRUE((bool)unit);
  EXPECT_EQ(unit->GetProducer(), eProducerSwift);
  EXPECT_EQ(unit->GetProducerVersion(), llvm::VersionTuple(1300, 0, 31, 1));
}
