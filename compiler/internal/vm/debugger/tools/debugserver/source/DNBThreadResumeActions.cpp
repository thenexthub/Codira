//===-- DNBThreadResumeActions.cpp ------------------------------*- C++ -*-===//
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
//  Created by Greg Clayton on 03/13/2010
//
//===----------------------------------------------------------------------===//

#include "DNBThreadResumeActions.h"

DNBThreadResumeActions::DNBThreadResumeActions()
    : m_actions(), m_signal_handled() {}

DNBThreadResumeActions::DNBThreadResumeActions(
    const DNBThreadResumeAction *actions, size_t num_actions)
    : m_actions(), m_signal_handled() {
  if (actions && num_actions) {
    m_actions.assign(actions, actions + num_actions);
    m_signal_handled.assign(num_actions, false);
  }
}

DNBThreadResumeActions::DNBThreadResumeActions(nub_state_t default_action,
                                               int signal)
    : m_actions(), m_signal_handled() {
  SetDefaultThreadActionIfNeeded(default_action, signal);
}

void DNBThreadResumeActions::Append(const DNBThreadResumeAction &action) {
  m_actions.push_back(action);
  m_signal_handled.push_back(false);
}

void DNBThreadResumeActions::AppendAction(nub_thread_t tid, nub_state_t state,
                                          int signal, nub_addr_t addr) {
  DNBThreadResumeAction action = {tid, state, signal, addr};
  Append(action);
}

const DNBThreadResumeAction *
DNBThreadResumeActions::GetActionForThread(nub_thread_t tid,
                                           bool default_ok) const {
  const size_t num_actions = m_actions.size();
  for (size_t i = 0; i < num_actions; ++i) {
    if (m_actions[i].tid == tid)
      return &m_actions[i];
  }
  if (default_ok && tid != INVALID_NUB_THREAD)
    return GetActionForThread(INVALID_NUB_THREAD, false);
  return NULL;
}

size_t DNBThreadResumeActions::NumActionsWithState(nub_state_t state) const {
  size_t count = 0;
  const size_t num_actions = m_actions.size();
  for (size_t i = 0; i < num_actions; ++i) {
    if (m_actions[i].state == state)
      ++count;
  }
  return count;
}

bool DNBThreadResumeActions::SetDefaultThreadActionIfNeeded(nub_state_t action,
                                                            int signal) {
  if (GetActionForThread(INVALID_NUB_THREAD, true) == NULL) {
    // There isn't a default action so we do need to set it.
    DNBThreadResumeAction default_action = {INVALID_NUB_THREAD, action, signal,
                                            INVALID_NUB_ADDRESS};
    m_actions.push_back(default_action);
    m_signal_handled.push_back(false);
    return true; // Return true as we did add the default action
  }
  return false;
}

void DNBThreadResumeActions::SetSignalHandledForThread(nub_thread_t tid) const {
  if (tid != INVALID_NUB_THREAD) {
    const size_t num_actions = m_actions.size();
    for (size_t i = 0; i < num_actions; ++i) {
      if (m_actions[i].tid == tid)
        m_signal_handled[i] = true;
    }
  }
}
