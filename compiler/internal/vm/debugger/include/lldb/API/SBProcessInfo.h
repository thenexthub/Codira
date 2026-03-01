//===-- SBProcessInfo.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBPROCESSINFO_H
#define LLDB_API_SBPROCESSINFO_H

#include "lldb/API/SBDefines.h"

namespace lldb {

class LLDB_API SBProcessInfo {
public:
  SBProcessInfo();
  SBProcessInfo(const SBProcessInfo &rhs);

  ~SBProcessInfo();

  SBProcessInfo &operator=(const SBProcessInfo &rhs);

  explicit operator bool() const;

  bool IsValid() const;

  const char *GetName();

  SBFileSpec GetExecutableFile();

  lldb::pid_t GetProcessID();

  uint32_t GetUserID();

  uint32_t GetGroupID();

  bool UserIDIsValid();

  bool GroupIDIsValid();

  uint32_t GetEffectiveUserID();

  uint32_t GetEffectiveGroupID();

  bool EffectiveUserIDIsValid();

  bool EffectiveGroupIDIsValid();

  lldb::pid_t GetParentProcessID();

  /// Return the target triple (arch-vendor-os) for the described process.
  const char *GetTriple();

private:
  friend class SBProcess;
  friend class SBProcessInfoList;

  lldb_private::ProcessInstanceInfo &ref();

  void SetProcessInfo(const lldb_private::ProcessInstanceInfo &proc_info_ref);

  std::unique_ptr<lldb_private::ProcessInstanceInfo> m_opaque_up;
};

} // namespace lldb

#endif // LLDB_API_SBPROCESSINFO_H
