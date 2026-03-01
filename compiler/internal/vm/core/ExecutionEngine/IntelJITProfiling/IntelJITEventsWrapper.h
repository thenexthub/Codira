//===-- IntelJITEventsWrapper.h - Intel JIT Events API Wrapper --*- C++ -*-===//
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
// This file defines a wrapper for the Intel JIT Events API. It allows for the
// implementation of the jitprofiling library to be swapped with an alternative
// implementation (for testing). To include this file, you must have the
// jitprofiling.h header available; it is available in Intel(R) VTune(TM)
// Amplifier XE 2011.
//
//===----------------------------------------------------------------------===//

#ifndef INTEL_JIT_EVENTS_WRAPPER_H
#define INTEL_JIT_EVENTS_WRAPPER_H

#include "jitprofiling.h"

namespace vm::core {

typedef enum {
  LoadBinaryModule,
  LoadBinarySection,
  UnloadBinaryModule,
  UnloadBinarySection
} IttEventType;

class IntelJITEventsWrapper {
  // Function pointer types for testing implementation of Intel jitprofiling
  // library
  typedef int (*NotifyEventPtr)(iJIT_JVM_EVENT, void*);
  typedef int (*IttnotifyInfoPtr)(IttEventType, const char *, unsigned int);
  typedef void (*RegisterCallbackExPtr)(void *, iJIT_ModeChangedEx );
  typedef iJIT_IsProfilingActiveFlags (*IsProfilingActivePtr)(void);
  typedef void (*FinalizeThreadPtr)(void);
  typedef void (*FinalizeProcessPtr)(void);
  typedef unsigned int (*GetNewMethodIDPtr)(void);

  NotifyEventPtr NotifyEventFunc;
  IttnotifyInfoPtr IttnotifyInfoFunc;
  RegisterCallbackExPtr RegisterCallbackExFunc;
  IsProfilingActivePtr IsProfilingActiveFunc;
  GetNewMethodIDPtr GetNewMethodIDFunc;

public:
  bool isAmplifierRunning() {
    return iJIT_IsProfilingActive() == iJIT_SAMPLING_ON;
  }

  IntelJITEventsWrapper()
      : NotifyEventFunc(::iJIT_NotifyEvent), IttnotifyInfoFunc(0),
        RegisterCallbackExFunc(::iJIT_RegisterCallbackEx),
        IsProfilingActiveFunc(::iJIT_IsProfilingActive),
        GetNewMethodIDFunc(::iJIT_GetNewMethodID) {}

  IntelJITEventsWrapper(NotifyEventPtr NotifyEventImpl,
                        IttnotifyInfoPtr IttnotifyInfoImpl,
                        RegisterCallbackExPtr RegisterCallbackExImpl,
                        IsProfilingActivePtr IsProfilingActiveImpl,
                        FinalizeThreadPtr FinalizeThreadImpl,
                        FinalizeProcessPtr FinalizeProcessImpl,
                        GetNewMethodIDPtr GetNewMethodIDImpl)
      : NotifyEventFunc(NotifyEventImpl), IttnotifyInfoFunc(IttnotifyInfoImpl),
        RegisterCallbackExFunc(RegisterCallbackExImpl),
        IsProfilingActiveFunc(IsProfilingActiveImpl),
        GetNewMethodIDFunc(GetNewMethodIDImpl) {}

  // Sends an event announcing that a function has been emitted
  //   return values are event-specific.  See Intel documentation for details.
  int  iJIT_NotifyEvent(iJIT_JVM_EVENT EventType, void *EventSpecificData) {
    if (!NotifyEventFunc)
      return -1;
    return NotifyEventFunc(EventType, EventSpecificData);
  }

  int iJitIttNotifyInfo(IttEventType EventType, const char *Name,
                        unsigned int Size) {
    if (!IttnotifyInfoFunc)
      return -1;
    return IttnotifyInfoFunc(EventType, Name, Size);
  }

  // Registers a callback function to receive notice of profiling state changes
  void iJIT_RegisterCallbackEx(void *UserData,
                               iJIT_ModeChangedEx NewModeCallBackFuncEx) {
    if (RegisterCallbackExFunc)
      RegisterCallbackExFunc(UserData, NewModeCallBackFuncEx);
  }

  // Returns the current profiler mode
  iJIT_IsProfilingActiveFlags iJIT_IsProfilingActive(void) {
    if (!IsProfilingActiveFunc)
      return iJIT_NOTHING_RUNNING;
    return IsProfilingActiveFunc();
  }

  // Generates a locally unique method ID for use in code registration
  unsigned int iJIT_GetNewMethodID(void) {
    if (!GetNewMethodIDFunc)
      return -1;
    return GetNewMethodIDFunc();
  }
};

} // namespace vm::core

#endif //INTEL_JIT_EVENTS_WRAPPER_H
