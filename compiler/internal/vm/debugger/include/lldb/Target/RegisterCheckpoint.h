//===-- RegisterCheckpoint.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_TARGET_REGISTERCHECKPOINT_H
#define LLDB_TARGET_REGISTERCHECKPOINT_H

#include "lldb/Target/StackID.h"
#include "lldb/Utility/UserID.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

// Inherit from UserID in case pushing/popping all register values can be done
// using a 64 bit integer that holds a baton/cookie instead of actually having
// to read all register values into a buffer
class RegisterCheckpoint : public UserID {
public:
  enum class Reason {
    // An expression is about to be run on the thread if the protocol that
    // talks to the debuggee supports checkpointing the registers using a
    // push/pop then the UserID base class in the RegisterCheckpoint can be
    // used to store the baton/cookie that refers to the remote saved state.
    eExpression,
    // The register checkpoint wants the raw register bytes, so they must be
    // read into m_data_sp, or the save/restore checkpoint should fail.
    eDataBackup
  };

  RegisterCheckpoint(Reason reason) : UserID(0), m_reason(reason) {}

  ~RegisterCheckpoint() = default;

  lldb::WritableDataBufferSP &GetData() { return m_data_sp; }

  const lldb::WritableDataBufferSP &GetData() const { return m_data_sp; }

protected:
  lldb::WritableDataBufferSP m_data_sp;
  Reason m_reason;

  // Make RegisterCheckpointSP if you wish to share the data in this class.
  RegisterCheckpoint(const RegisterCheckpoint &) = delete;
  const RegisterCheckpoint &operator=(const RegisterCheckpoint &) = delete;
};

} // namespace lldb_private

#endif // LLDB_TARGET_REGISTERCHECKPOINT_H
