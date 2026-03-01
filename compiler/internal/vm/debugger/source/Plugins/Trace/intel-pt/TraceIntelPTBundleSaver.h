//===-- TraceIntelPTBundleSaver.h ----------------------------*- C++ //-*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TRACEINTELPTBUNDLESAVER_H
#define LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TRACEINTELPTBUNDLESAVER_H

#include "TraceIntelPT.h"
#include "TraceIntelPTJSONStructs.h"

namespace lldb_private {
namespace trace_intel_pt {

class TraceIntelPTBundleSaver {
public:
  /// Save the Intel PT trace of a live process to the specified directory,
  /// which will be created if needed. This will also create a file
  /// \a <directory>/trace.json with the description of the trace
  /// bundle, along with others files which contain the actual trace data.
  /// The trace.json file can be used later as input for the "trace load"
  /// command to load the trace in LLDB.
  ///
  /// \param[in] trace_ipt
  ///     The Intel PT trace to be saved to disk.
  ///
  /// \param[in] directory
  ///     The directory where the trace bundle will be created.
  ///
  /// \param[in] compact
  ///     Filter out information irrelevant to the traced processes in the
  ///     context switch and intel pt traces when using per-cpu mode. This
  ///     effectively reduces the size of those traces.
  ///
  /// \return
  ///   A \a FileSpec pointing to the bundle description file, or an \a
  ///   llvm::Error otherwise.
  llvm::Expected<FileSpec> SaveToDisk(TraceIntelPT &trace_ipt,
                                      FileSpec directory, bool compact);
};

} // namespace trace_intel_pt
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TRACEINTELPTBUNDLESAVER_H
