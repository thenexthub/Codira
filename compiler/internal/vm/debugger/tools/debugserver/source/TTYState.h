//===-- TTYState.h ----------------------------------------------*- C++ -*-===//
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
//
//  Created by Greg Clayton on 3/26/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_TTYSTATE_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_TTYSTATE_H

#include <cstdint>
#include <termios.h>

class TTYState {
public:
  TTYState();
  ~TTYState();

  bool GetTTYState(int fd, bool saveProcessGroup);
  bool SetTTYState() const;

  bool IsValid() const {
    return FileDescriptorValid() && TFlagsValid() && TTYStateValid();
  }
  bool FileDescriptorValid() const { return m_fd >= 0; }
  bool TFlagsValid() const { return m_tflags != -1; }
  bool TTYStateValid() const { return m_ttystateErr == 0; }
  bool ProcessGroupValid() const { return m_processGroup != -1; }

protected:
  int m_fd; // File descriptor
  int m_tflags;
  int m_ttystateErr;
  struct termios m_ttystate;
  pid_t m_processGroup;
};

class TTYStateSwitcher {
public:
  TTYStateSwitcher();
  ~TTYStateSwitcher();

  bool GetState(uint32_t idx, int fd, bool saveProcessGroup);
  bool SetState(uint32_t idx) const;
  uint32_t NumStates() const { return sizeof(m_ttystates) / sizeof(TTYState); }
  bool ValidStateIndex(uint32_t idx) const { return idx < NumStates(); }

protected:
  mutable uint32_t m_currentState;
  TTYState m_ttystates[2];
};

#endif
