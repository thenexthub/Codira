//===-- SBUnixSignals.h -----------------------------------------------*- C++
//-*-===//
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

#ifndef LLDB_API_SBUNIXSIGNALS_H
#define LLDB_API_SBUNIXSIGNALS_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBUnixSignals {
public:
  SBUnixSignals();

  SBUnixSignals(const lldb::SBUnixSignals &rhs);

  ~SBUnixSignals();

  const SBUnixSignals &operator=(const lldb::SBUnixSignals &rhs);

  void Clear();

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetSignalAsCString(int32_t signo) const;

  int32_t GetSignalNumberFromName(const char *name) const;

  bool GetShouldSuppress(int32_t signo) const;

  bool SetShouldSuppress(int32_t signo, bool value);

  bool GetShouldStop(int32_t signo) const;

  bool SetShouldStop(int32_t signo, bool value);

  bool GetShouldNotify(int32_t signo) const;

  bool SetShouldNotify(int32_t signo, bool value);

  int32_t GetNumSignals() const;

  int32_t GetSignalAtIndex(int32_t index) const;

protected:
  friend class SBProcess;
  friend class SBPlatform;

  SBUnixSignals(lldb::ProcessSP &process_sp);

  SBUnixSignals(lldb::PlatformSP &platform_sp);

  lldb::UnixSignalsSP GetSP() const;

  void SetSP(const lldb::UnixSignalsSP &signals_sp);

private:
  lldb::UnixSignalsWP m_opaque_wp;
};

} // namespace lldb

#endif // LLDB_API_SBUNIXSIGNALS_H
