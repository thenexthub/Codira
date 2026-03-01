//===-- PThreadCondition.h --------------------------------------*- C++ -*-===//
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
//  Created by Greg Clayton on 6/16/07.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_DEBUGSERVER_SOURCE_PTHREADCONDITION_H
#define LLDB_TOOLS_DEBUGSERVER_SOURCE_PTHREADCONDITION_H

#include <pthread.h>

class PThreadCondition {
public:
  PThreadCondition() { ::pthread_cond_init(&m_condition, NULL); }

  ~PThreadCondition() { ::pthread_cond_destroy(&m_condition); }

  pthread_cond_t *Condition() { return &m_condition; }

  int Broadcast() { return ::pthread_cond_broadcast(&m_condition); }

  int Signal() { return ::pthread_cond_signal(&m_condition); }

protected:
  pthread_cond_t m_condition;
};

#endif
