//===--- ExecutorBridge.h - C++ side of executor bridge -------------------===//
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

#ifndef SWIFT_EXECUTOR_BRIDGE_H_
#define SWIFT_EXECUTOR_BRIDGE_H_

#include "language/Runtime/Concurrency.h"

namespace language {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

extern "C" SWIFT_CC(swift)
SerialExecutorRef swift_getMainExecutor();

#if !SWIFT_CONCURRENCY_EMBEDDED
extern "C" SWIFT_CC(swift)
void *swift_createDispatchEventC(void (*handler)(void *), void *context);

extern "C" SWIFT_CC(swift)
void swift_destroyDispatchEventC(void *event);

extern "C" SWIFT_CC(swift)
void swift_signalDispatchEvent(void *event);
#endif // !SWIFT_CONCURRENCY_EMBEDDED

extern "C" SWIFT_CC(swift) __attribute__((noreturn))
void swift_dispatchMain();

extern "C" SWIFT_CC(swift)
void swift_createDefaultExecutors();

extern "C" SWIFT_CC(swift)
void swift_createDefaultExecutorsOnce();

#pragma clang diagnostic pop

} // namespace language

#endif /* SWIFT_EXECUTOR_BRIDGE_H_ */
