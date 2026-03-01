//===-- SymbolsTest.cpp ---------------------------------------------------===//
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

#include "TestingSupport/SubsystemRAII.h"
#include "lldb/Core/ModuleSpec.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostInfo.h"
#include "lldb/Target/Target.h"

using namespace lldb_private;

namespace {
class SymbolsTest : public ::testing::Test {
public:
  SubsystemRAII<FileSystem, HostInfo> subsystems;
};
} // namespace

TEST_F(
    SymbolsTest,
    TerminateLocateExecutableSymbolFileForUnknownExecutableAndUnknownSymbolFile) {
  ModuleSpec module_spec;
  FileSpecList search_paths = Target::GetDefaultDebugFileSearchPaths();
  StatisticsMap map;
  FileSpec symbol_file_spec =
      PluginManager::LocateExecutableSymbolFile(module_spec, search_paths, map);
  EXPECT_TRUE(symbol_file_spec.GetFilename().IsEmpty());
}

TEST_F(SymbolsTest,
       LocateExecutableSymbolFileForUnknownExecutableAndMissingSymbolFile) {
  ModuleSpec module_spec;
  // using a GUID here because the symbol file shouldn't actually exist on disk
  module_spec.GetSymbolFileSpec().SetFile(
      "4A524676-B24B-4F4E-968A-551D465EBAF1.so", FileSpec::Style::native);
  FileSpecList search_paths = Target::GetDefaultDebugFileSearchPaths();
  StatisticsMap map;
  FileSpec symbol_file_spec =
      PluginManager::LocateExecutableSymbolFile(module_spec, search_paths, map);
  EXPECT_TRUE(symbol_file_spec.GetFilename().IsEmpty());
}
