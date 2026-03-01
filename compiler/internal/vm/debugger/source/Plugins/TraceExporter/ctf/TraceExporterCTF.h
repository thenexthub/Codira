//===-- TraceExporterCTF.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_TRACE_EXPORTER_CTF_H
#define LLDB_SOURCE_PLUGINS_TRACE_EXPORTER_CTF_H

#include "lldb/Target/TraceExporter.h"

namespace lldb_private {
namespace ctf {

/// Trace Exporter Plugin that can produce traces in Chrome Trace Format.
/// Still in development.
class TraceExporterCTF : public TraceExporter {
public:
  ~TraceExporterCTF() override = default;

  /// PluginInterface protocol
  /// \{
  static llvm::Expected<lldb::TraceExporterUP> CreateInstance();

  llvm::StringRef GetPluginName() override {
    return GetPluginNameStatic();
  }

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "ctf"; }
  /// \}
};

} // namespace ctf
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_TRACE_EXPORTER_CTF_H
