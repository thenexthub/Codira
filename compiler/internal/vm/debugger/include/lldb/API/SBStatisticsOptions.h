//===-- SBStatisticsOptions.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBSTATISTICSOPTIONS_H
#define LLDB_API_SBSTATISTICSOPTIONS_H

#include "lldb/API/SBDefines.h"

namespace lldb {

/// This class handles the verbosity when dumping statistics
class LLDB_API SBStatisticsOptions {
public:
  SBStatisticsOptions();
  SBStatisticsOptions(const lldb::SBStatisticsOptions &rhs);
  ~SBStatisticsOptions();

  const SBStatisticsOptions &operator=(const lldb::SBStatisticsOptions &rhs);

  /// If true, dump only high-level summary statistics. Exclude details like
  /// targets, modules, breakpoints, etc. This turns off `IncludeTargets`,
  /// `IncludeModules` and `IncludeTranscript` by default.
  ///
  /// Defaults to false.
  void SetSummaryOnly(bool b);
  bool GetSummaryOnly();

  /// If true, dump statistics for the targets, including breakpoints,
  /// expression evaluations, frame variables, etc.
  ///
  /// Defaults to true, unless the `SummaryOnly` mode is enabled, in which case
  /// this is turned off unless specified.
  ///
  /// If both `IncludeTargets` and `IncludeModules` are true, a list of module
  /// identifiers will be added to the "targets" section.
  void SetIncludeTargets(bool b);
  bool GetIncludeTargets() const;

  /// If true, dump statistics for the modules, including time and size of
  /// various aspects of the module and debug information, type system, path,
  /// etc.
  ///
  /// Defaults to true, unless the `SummaryOnly` mode is enabled, in which case
  /// this is turned off unless specified.
  ///
  /// If both `IncludeTargets` and `IncludeModules` are true, a list of module
  /// identifiers will be added to the "targets" section.
  void SetIncludeModules(bool b);
  bool GetIncludeModules() const;

  /// If true and the setting `interpreter.save-transcript` is enabled, include
  /// a JSON array with all commands the user and/or scripts executed during a
  /// debug session.
  ///
  /// Defaults to false.
  void SetIncludeTranscript(bool b);
  bool GetIncludeTranscript() const;

  /// If set to true, the debugger will load all debug info that is available
  /// and report statistics on the total amount. If this is set to false, then
  /// only report statistics on the currently loaded debug information.
  /// This can avoid loading debug info from separate files just so it can
  /// report the total size which can slow down statistics reporting.
  void SetReportAllAvailableDebugInfo(bool b);
  bool GetReportAllAvailableDebugInfo();

protected:
  friend class SBTarget;
  const lldb_private::StatisticsOptions &ref() const;

private:
  std::unique_ptr<lldb_private::StatisticsOptions> m_opaque_up;
};
} // namespace lldb
#endif // LLDB_API_SBSTATISTICSOPTIONS_H
