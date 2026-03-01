//===-- SBReproducer.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBREPRODUCER_H
#define LLDB_API_SBREPRODUCER_H

#include "lldb/API/SBDefines.h"

namespace lldb {

#ifndef SWIG
class LLDB_API SBReplayOptions {
public:
  SBReplayOptions();
  SBReplayOptions(const SBReplayOptions &rhs);
  ~SBReplayOptions();

  SBReplayOptions &operator=(const SBReplayOptions &rhs);

  void SetVerify(bool verify);
  bool GetVerify() const;

  void SetCheckVersion(bool check);
  bool GetCheckVersion() const;
};
#endif

/// The SBReproducer class is special because it bootstraps the capture and
/// replay of SB API calls. As a result we cannot rely on any other SB objects
/// in the interface or implementation of this class.
class LLDB_API SBReproducer {
public:
#ifndef SWIG
  static const char *Capture();
#endif
  static const char *Capture(const char *path);
#ifndef SWIG
  static const char *Replay(const char *path);
  static const char *Replay(const char *path, bool skip_version_check);
  static const char *Replay(const char *path, const SBReplayOptions &options);
#endif
  static const char *PassiveReplay(const char *path);
#ifndef SWIG
  static const char *Finalize(const char *path);
  static const char *GetPath();
#endif
  static bool SetAutoGenerate(bool b);
#ifndef SWIG
  static bool Generate();
#endif

  /// The working directory is set to the current working directory when the
  /// reproducers are initialized. This method allows setting a different
  /// working directory. This is used by the API test suite  which temporarily
  /// changes the directory to where the test lives. This is a NO-OP in every
  /// mode but capture.
  static void SetWorkingDirectory(const char *path);
};

} // namespace lldb

#endif
