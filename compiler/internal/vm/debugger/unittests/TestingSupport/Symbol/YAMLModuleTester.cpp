//===-- YAMLModuleTester.cpp ----------------------------------------------===//
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

#include "TestingSupport/Symbol/YAMLModuleTester.h"
#include "Plugins/SymbolFile/DWARF/DWARFDebugInfo.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "lldb/Core/Section.h"
#include "llvm/ObjectYAML/DWARFEmitter.h"

using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

YAMLModuleTester::YAMLModuleTester(llvm::StringRef yaml_data, size_t cu_index) {
  llvm::Expected<TestFile> File = TestFile::fromYaml(yaml_data);
  EXPECT_THAT_EXPECTED(File, llvm::Succeeded());
  m_file = std::move(*File);

  m_module_sp = std::make_shared<Module>(m_file->moduleSpec());
  auto &symfile = *llvm::cast<SymbolFileDWARF>(m_module_sp->GetSymbolFile());

  m_dwarf_unit = symfile.DebugInfo().GetUnitAtIndex(cu_index);
}
