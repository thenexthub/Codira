//===--- Tracing.cpp ------------------------------------------------------===//
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

#include "SourceKit/Support/Tracing.h"

#include "toolchain/Support/Mutex.h"
#include "toolchain/Support/YAMLTraits.h"

using namespace SourceKit;
using namespace toolchain;
using language::OptionSet;

static toolchain::sys::Mutex consumersLock;
// Must hold consumersLock to access.
static std::vector<trace::TraceConsumer *> consumers;
// Must hold consumersLock to modify, but can be read any time.
static std::atomic<uint64_t> tracedOperations;

//===----------------------------------------------------------------------===//
// Trace commands
//===----------------------------------------------------------------------===//

// Is tracing enabled
bool trace::anyEnabled() { return static_cast<bool>(tracedOperations); }

bool trace::enabled(OperationKind op) {
  return OptionSet<OperationKind>(tracedOperations).contains(op);
}

// Trace start of perform sema call, returns OpId
uint64_t trace::startOperation(trace::OperationKind OpKind,
                               const trace::CodiraInvocation &Inv,
                               const trace::StringPairs &OpArgs) {
  static std::atomic<uint64_t> operationId(0);
  auto OpId = ++operationId;
  if (trace::anyEnabled()) {
    toolchain::sys::ScopedLock L(consumersLock);
    for (auto *consumer : consumers) {
      consumer->operationStarted(OpId, OpKind, Inv, OpArgs);
    }
  }
  return OpId;
}

// Operation previously started with startXXX has finished
void trace::operationFinished(uint64_t OpId, trace::OperationKind OpKind,
                              ArrayRef<DiagnosticEntryInfo> Diagnostics) {
  if (trace::anyEnabled()) {
    toolchain::sys::ScopedLock L(consumersLock);
    for (auto *consumer : consumers) {
      consumer->operationFinished(OpId, OpKind, Diagnostics);
    }
  }
}

// Must be called with consumersLock held.
static void updateTracedOperations() {
  OptionSet<trace::OperationKind> operations;
  for (auto *consumer : consumers) {
    operations |= consumer->desiredOperations();
  }
  // It is safe to store without a compare, because writers hold consumersLock.
  tracedOperations.store(uint64_t(operations), std::memory_order_relaxed);
}

// Register trace consumer
void trace::registerConsumer(trace::TraceConsumer *Consumer) {
  toolchain::sys::ScopedLock L(consumersLock);
  consumers.push_back(Consumer);
  updateTracedOperations();
}

void trace::unregisterConsumer(trace::TraceConsumer *Consumer) {
  toolchain::sys::ScopedLock L(consumersLock);
  consumers.erase(std::remove(consumers.begin(), consumers.end(), Consumer),
                  consumers.end());
  updateTracedOperations();
}
