//===--- ExecutorImpl.cpp - C++ side of executor impl ---------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#if SWIFT_CONCURRENCY_USES_DISPATCH
#include <dispatch/dispatch.h>
#endif

#include "Error.h"
#include "ExecutorBridge.h"

using namespace language;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"


extern "C" SWIFT_CC(swift)
void _swift_task_checkIsolatedSwift(
  HeapObject *executor,
  const Metadata *executorType,
  const SerialExecutorWitnessTable *witnessTable
);

extern "C" SWIFT_CC(swift) bool _swift_task_isMainExecutorSwift(
    HeapObject *executor, const Metadata *executorType,
    const SerialExecutorWitnessTable *witnessTable);

extern "C" SWIFT_CC(swift) void swift_task_checkIsolatedImpl(
    SerialExecutorRef executor) {
  HeapObject *identity = executor.getIdentity();

  // We might be being called with an actor rather than a "proper"
  // SerialExecutor; in that case, we won't have a SerialExecutor witness
  // table.
  if (executor.hasSerialExecutorWitnessTable()) {
    _swift_task_checkIsolatedSwift(identity, swift_getObjectType(identity),
                                   executor.getSerialExecutorWitnessTable());
  } else {
    const Metadata *objectType = swift_getObjectType(executor.getIdentity());
    auto typeName = swift_getTypeName(objectType, true);

    swift_Concurrency_fatalError(
        0, "Incorrect actor executor assumption; expected '%.*s' executor.\n",
        (int)typeName.length, typeName.data);
  }
}


extern "C" SWIFT_CC(swift)
bool _swift_task_isIsolatingCurrentContextSwift(
  HeapObject *executor,
  const Metadata *executorType,
  const SerialExecutorWitnessTable *witnessTable
);

extern "C" SWIFT_CC(swift) bool swift_task_isIsolatingCurrentContextImpl(
    SerialExecutorRef executor) {
  HeapObject *identity = executor.getIdentity();

  // We might be being called with an actor rather than a "proper"
  // SerialExecutor; in that case, we won't have a SerialExecutor witness
  // table.
  if (executor.hasSerialExecutorWitnessTable()) {
    return _swift_task_isIsolatingCurrentContextSwift(
        identity, swift_getObjectType(identity),
        executor.getSerialExecutorWitnessTable());
  } else {
    const Metadata *objectType = swift_getObjectType(executor.getIdentity());
    auto typeName = swift_getTypeName(objectType, true);

    swift_Concurrency_fatalError(
        0, "Incorrect actor executor assumption; expected '%.*s' executor.\n",
        (int)typeName.length, typeName.data);
  }
}

extern "C" SWIFT_CC(swift) bool swift_task_isMainExecutorImpl(
    SerialExecutorRef executor) {
  HeapObject *identity = executor.getIdentity();
  return executor.hasSerialExecutorWitnessTable() &&
         _swift_task_isMainExecutorSwift(
             identity, swift_getObjectType(identity),
             executor.getSerialExecutorWitnessTable());
}
