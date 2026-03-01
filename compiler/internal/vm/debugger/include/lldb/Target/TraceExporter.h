//===-- TraceExporter.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_TRACE_EXPORTER_H
#define LLDB_TARGET_TRACE_EXPORTER_H

#include "lldb/Core/PluginInterface.h"
#include "lldb/lldb-forward.h"
#include "llvm/Support/Error.h"

namespace lldb_private {

/// \class TraceExporter TraceExporter.h "lldb/Target/TraceExporter.h"
/// A plug-in interface definition class for trace exporters.
///
/// Trace exporter plug-ins operate on traces, converting the trace data
/// provided by an \a lldb_private::TraceCursor into a different format that can
/// be digested by other tools, e.g. Chrome Trace Event Profiler.
///
/// Trace exporters are supposed to operate on an architecture-agnostic fashion,
/// as a TraceCursor, which feeds the data, hides the actual trace technology
/// being used.
class TraceExporter : public PluginInterface {
public:
  /// Create an instance of a trace exporter plugin given its name.
  ///
  /// \param[in] plugin_Name
  ///     Plug-in name to search.
  ///
  /// \return
  ///     A \a TraceExporterUP instance, or an \a llvm::Error if the plug-in
  ///     name doesn't match any registered plug-ins.
  static llvm::Expected<lldb::TraceExporterUP>
  FindPlugin(llvm::StringRef plugin_name);
};

} // namespace lldb_private

#endif // LLDB_TARGET_TRACE_EXPORTER_H
