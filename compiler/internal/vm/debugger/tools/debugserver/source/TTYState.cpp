//===-- TTYState.cpp --------------------------------------------*- C++ -*-===//
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

#include "TTYState.h"
#include <fcntl.h>
#include <sys/signal.h>
#include <unistd.h>

TTYState::TTYState()
    : m_fd(-1), m_tflags(-1), m_ttystateErr(-1), m_processGroup(-1) {}

TTYState::~TTYState() = default;

bool TTYState::GetTTYState(int fd, bool saveProcessGroup) {
  if (fd >= 0 && ::isatty(fd)) {
    m_fd = fd;
    m_tflags = fcntl(fd, F_GETFL, 0);
    m_ttystateErr = tcgetattr(fd, &m_ttystate);
    if (saveProcessGroup)
      m_processGroup = tcgetpgrp(0);
    else
      m_processGroup = -1;
  } else {
    m_fd = -1;
    m_tflags = -1;
    m_ttystateErr = -1;
    m_processGroup = -1;
  }
  return m_ttystateErr == 0;
}

bool TTYState::SetTTYState() const {
  if (IsValid()) {
    if (TFlagsValid())
      fcntl(m_fd, F_SETFL, m_tflags);

    if (TTYStateValid())
      tcsetattr(m_fd, TCSANOW, &m_ttystate);

    if (ProcessGroupValid()) {
      // Save the original signal handler.
      void (*saved_sigttou_callback)(int) = NULL;
      saved_sigttou_callback = (void (*)(int))signal(SIGTTOU, SIG_IGN);
      // Set the process group
      tcsetpgrp(m_fd, m_processGroup);
      // Restore the original signal handler.
      signal(SIGTTOU, saved_sigttou_callback);
    }
    return true;
  }
  return false;
}

TTYStateSwitcher::TTYStateSwitcher() : m_currentState(~0) {}

TTYStateSwitcher::~TTYStateSwitcher() = default;

bool TTYStateSwitcher::GetState(uint32_t idx, int fd, bool saveProcessGroup) {
  if (ValidStateIndex(idx))
    return m_ttystates[idx].GetTTYState(fd, saveProcessGroup);
  return false;
}

bool TTYStateSwitcher::SetState(uint32_t idx) const {
  if (!ValidStateIndex(idx))
    return false;

  // See if we already are in this state?
  if (ValidStateIndex(m_currentState) && (idx == m_currentState) &&
      m_ttystates[idx].IsValid())
    return true;

  // Set the state to match the index passed in and only update the
  // current state if there are no errors.
  if (m_ttystates[idx].SetTTYState()) {
    m_currentState = idx;
    return true;
  }

  // We failed to set the state. The tty state was invalid or not
  // initialized.
  return false;
}
