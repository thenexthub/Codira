//===-- SBProgress.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBPROGRESS_H
#define LLDB_API_SBPROGRESS_H

#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBDefines.h"

namespace lldb {

/// A Progress indicator helper class.
///
/// Any potentially long running sections of code in LLDB should report
/// progress so that clients are aware of delays that might appear during
/// debugging. Delays commonly include indexing debug information, parsing
/// symbol tables for object files, downloading symbols from remote
/// repositories, and many more things.
///
/// The Progress class helps make sure that progress is correctly reported
/// and will always send an initial progress update, updates when
/// Progress::Increment() is called, and also will make sure that a progress
/// completed update is reported even if the user doesn't explicitly cause one
/// to be sent.
class LLDB_API SBProgress {
public:
  /// Construct a progress object with a title, details and a given debugger.
  /// \param title
  ///   The title of the progress object.
  /// \param details
  ///   The details of the progress object.
  /// \param debugger
  ///   The debugger for this progress object to report to.
  SBProgress(const char *title, const char *details, SBDebugger &debugger);

  /// Construct a progress object with a title, details, the total units of work
  /// to be done, and a given debugger.
  /// \param title
  ///   The title of the progress object.
  /// \param details
  ///   The details of the progress object.
  /// \param total_units
  ///   The total number of units of work to be done.
  /// \param debugger
  ///   The debugger for this progress object to report to.
  SBProgress(const char *title, const char *details, uint64_t total_units,
             SBDebugger &debugger);

#ifndef SWIG
  SBProgress(SBProgress &&rhs);
#endif

  ~SBProgress();

  void Increment(uint64_t amount, const char *description = nullptr);

  /// Explicitly finalize an SBProgress, this can be used to terminate a
  /// progress on command instead of waiting for a garbage collection or other
  /// RAII to destroy the contained progress object.
  void Finalize();

protected:
  lldb_private::Progress &ref() const;

private:
  SBProgress(const SBProgress &rhs) = delete;
  const SBProgress &operator=(const SBProgress &rhs) = delete;

  std::unique_ptr<lldb_private::Progress> m_opaque_up;
}; // SBProgress
} // namespace lldb

#endif // LLDB_API_SBPROGRESS_H
