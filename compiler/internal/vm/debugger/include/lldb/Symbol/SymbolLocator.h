//===-- SymbolLocator.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_SYMBOLLOCATOR_H
#define LLDB_SYMBOL_SYMBOLLOCATOR_H

#include "lldb/Core/PluginInterface.h"
#include "lldb/Utility/UUID.h"

namespace lldb_private {

class SymbolLocator : public PluginInterface {
public:
  SymbolLocator() = default;

  /// Locate the symbol file for the given UUID on a background thread. This
  /// function returns immediately. Under the hood it uses the debugger's
  /// thread pool to call DownloadObjectAndSymbolFile. If a symbol file is
  /// found, this will notify all target which contain the module with the
  /// given UUID.
  static void DownloadSymbolFileAsync(const UUID &uuid);
};

} // namespace lldb_private

#endif // LLDB_SYMBOL_SYMBOLFILELOCATOR_H
