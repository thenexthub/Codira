//===-- SymbolVendor.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_SYMBOLVENDOR_H
#define LLDB_SYMBOL_SYMBOLVENDOR_H

#include <vector>

#include "lldb/Core/ModuleChild.h"
#include "lldb/Core/PluginInterface.h"
#include "lldb/Symbol/SourceModule.h"
#include "lldb/Symbol/SymbolFile.h"
#include "lldb/Symbol/TypeMap.h"
#include "lldb/lldb-private.h"
#include "llvm/ADT/DenseSet.h"

namespace lldb_private {

// The symbol vendor class is designed to abstract the process of searching for
// debug information for a given module. Platforms can subclass this class and
// provide extra ways to find debug information. Examples would be a subclass
// that would allow for locating a stand alone debug file, parsing debug maps,
// or runtime data in the object files. A symbol vendor can use multiple
// sources (SymbolFile objects) to provide the information and only parse as
// deep as needed in order to provide the information that is requested.
class SymbolVendor : public ModuleChild, public PluginInterface {
public:
  static SymbolVendor *FindPlugin(const lldb::ModuleSP &module_sp,
                                  Stream *feedback_strm);

  // Constructors and Destructors
  SymbolVendor(const lldb::ModuleSP &module_sp);

  void AddSymbolFileRepresentation(const lldb::ObjectFileSP &objfile_sp);

  SymbolFile *GetSymbolFile() { return m_sym_file_up.get(); }

  // PluginInterface protocol
  llvm::StringRef GetPluginName() override { return "vendor-default"; }

protected:
  std::unique_ptr<SymbolFile> m_sym_file_up; // A single symbol file. Subclasses
                                             // can add more of these if needed.
};

} // namespace lldb_private

#endif // LLDB_SYMBOL_SYMBOLVENDOR_H
