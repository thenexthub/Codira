//===- YAMLModuleTester.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UNITTESTS_TESTINGSUPPORT_SYMBOL_YAMLMODULETESTER_H
#define LLDB_UNITTESTS_TESTINGSUPPORT_SYMBOL_YAMLMODULETESTER_H

#include "Plugins/ObjectFile/ELF/ObjectFileELF.h"
#include "Plugins/SymbolFile/DWARF/DWARFUnit.h"
#include "Plugins/SymbolFile/DWARF/SymbolFileDWARF.h"
#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "TestingSupport/SubsystemRAII.h"
#include "TestingSupport/TestUtilities.h"
#include "lldb/Core/Module.h"
#include "lldb/Host/HostInfo.h"
#include <optional>

namespace lldb_private {

/// Helper class that can construct a module from YAML and evaluate
/// DWARF expressions on it.
class YAMLModuleTester {
protected:
  SubsystemRAII<FileSystem, HostInfo, TypeSystemClang, ObjectFileELF,
                plugin::dwarf::SymbolFileDWARF>
      subsystems;
  std::optional<TestFile> m_file;
  lldb::ModuleSP m_module_sp;
  plugin::dwarf::DWARFUnit *m_dwarf_unit;

public:
  /// Parse the debug info sections from the YAML description.
  YAMLModuleTester(llvm::StringRef yaml_data, size_t cu_index = 0);
  plugin::dwarf::DWARFUnit *GetDwarfUnit() const { return m_dwarf_unit; }
  lldb::ModuleSP GetModule() const { return m_module_sp; }
};

} // namespace lldb_private

#endif // LLDB_UNITTESTS_TESTINGSUPPORT_SYMBOL_YAMLMODULETESTER_H
