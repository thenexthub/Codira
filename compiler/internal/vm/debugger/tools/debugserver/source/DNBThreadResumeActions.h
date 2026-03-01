//===-- DNBThreadResumeActions.h --------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBTHREADRESUMEACTIONS_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBTHREADRESUMEACTIONS_H

#include <vector>

#include "DNBDefs.h"

class DNBThreadResumeActions {
public:
  DNBThreadResumeActions();

  DNBThreadResumeActions(nub_state_t default_action, int signal);

  DNBThreadResumeActions(const DNBThreadResumeAction *actions,
                         size_t num_actions);

  bool IsEmpty() const { return m_actions.empty(); }

  void Append(const DNBThreadResumeAction &action);

  void AppendAction(nub_thread_t tid, nub_state_t state, int signal = 0,
                    nub_addr_t addr = INVALID_NUB_ADDRESS);

  void AppendResumeAll() { AppendAction(INVALID_NUB_THREAD, eStateRunning); }

  void AppendSuspendAll() { AppendAction(INVALID_NUB_THREAD, eStateStopped); }

  void AppendStepAll() { AppendAction(INVALID_NUB_THREAD, eStateStepping); }

  const DNBThreadResumeAction *GetActionForThread(nub_thread_t tid,
                                                  bool default_ok) const;

  size_t NumActionsWithState(nub_state_t state) const;

  bool SetDefaultThreadActionIfNeeded(nub_state_t action, int signal);

  void SetSignalHandledForThread(nub_thread_t tid) const;

  const DNBThreadResumeAction *GetFirst() const { return m_actions.data(); }

  size_t GetSize() const { return m_actions.size(); }

  void Clear() {
    m_actions.clear();
    m_signal_handled.clear();
  }

protected:
  std::vector<DNBThreadResumeAction> m_actions;
  mutable std::vector<bool> m_signal_handled;
};

#endif // LLDB_TOOLS_DEBUGSERVER_SOURCE_DNBTHREADRESUMEACTIONS_H
