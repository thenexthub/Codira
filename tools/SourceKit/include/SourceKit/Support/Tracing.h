//===--- Tracing.h - Tracing Interface --------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_SUPPORT_TRACING_H
#define TOOLCHAIN_SOURCEKIT_SUPPORT_TRACING_H

#include "SourceKit/Core/Toolchain.h"
#include "SourceKit/Core/LangSupport.h"
#include "SourceKit/Support/UIdent.h"
#include "language/Basic/OptionSet.h"
#include "toolchain/ADT/ArrayRef.h"
#include <optional>

#include <vector>

namespace SourceKit {
  struct DiagnosticEntryInfo;

namespace trace {

struct CodiraArguments {
  std::string PrimaryFile;
  std::string Arguments;
};

enum class OperationKind : uint64_t {
  PerformSema = 1 << 0,
  IndexSource = 1 << 1,
  CodeCompletion = 1 << 2,

  Last = CodeCompletion,
  All = (Last << 1) - 1
};

typedef std::vector<std::pair<std::string, std::string>> StringPairs;

struct CodiraInvocation {
  CodiraArguments Args;
};
  
class TraceConsumer {
public:
  virtual ~TraceConsumer() = default;

  // Trace start of SourceKit operation
  virtual void operationStarted(uint64_t OpId, OperationKind OpKind,
                                const CodiraInvocation &Inv,
                                const StringPairs &OpArgs) = 0;

  // Operation previously started with startXXX has finished
  virtual void operationFinished(uint64_t OpId, OperationKind OpKind,
                                 ArrayRef<DiagnosticEntryInfo> Diagnostics) = 0;

  /// Returns the set of operations this consumer is interested in.
  ///
  /// Note: this is only a hint. Implementations should check the operation kind
  /// if they need to.
  virtual language::OptionSet<OperationKind> desiredOperations() {
    return OperationKind::All;
  }
};

// Is tracing enabled
bool anyEnabled();

// Is tracing enabled for \p op.
bool enabled(OperationKind op);

// Trace start of SourceKit operation, returns OpId
uint64_t startOperation(OperationKind OpKind,
                        const CodiraInvocation &Inv,
                        const StringPairs &OpArgs = StringPairs());

// Operation previously started with startXXX has finished
void operationFinished(uint64_t OpId, OperationKind OpKind,
                       ArrayRef<DiagnosticEntryInfo> Diagnostics);

// Register trace consumer.
void registerConsumer(TraceConsumer *Consumer);

// Register trace consumer.
void unregisterConsumer(TraceConsumer *Consumer);

// Class that utilizes the RAII idiom for the operations being traced
class TracedOperation final {
  using DiagnosticProvider = std::function<void(SmallVectorImpl<DiagnosticEntryInfo> &)>;

  OperationKind OpKind;
  std::optional<uint64_t> OpId;
  std::optional<DiagnosticProvider> DiagProvider;
  bool Enabled;

public:
  TracedOperation(OperationKind OpKind) : OpKind(OpKind) {
    Enabled = trace::enabled(OpKind);
  }
  ~TracedOperation() {
    finish();
  }

  TracedOperation(TracedOperation &&) = delete;
  TracedOperation &operator=(TracedOperation &&) = delete;
  TracedOperation(const TracedOperation &) = delete;
  TracedOperation &operator=(const TracedOperation &) = delete;

  bool enabled() const { return Enabled; }

  void start(const CodiraInvocation &Inv,
             const StringPairs &OpArgs = StringPairs()) {
    assert(!OpId.has_value());
    OpId = startOperation(OpKind, Inv, OpArgs);
  }

  void finish() {
    if (OpId.has_value()) {
      SmallVector<DiagnosticEntryInfo, 8> Diagnostics;
      if (DiagProvider.has_value())
        (*DiagProvider)(Diagnostics);
      operationFinished(OpId.value(), OpKind, Diagnostics);
      OpId.reset();
    }
  }

  void setDiagnosticProvider(DiagnosticProvider &&DiagProvider) {
    assert(!this->DiagProvider.has_value());
    this->DiagProvider = std::move(DiagProvider);
  }
};

} // namespace sourcekitd
} // namespace trace

#endif /* defined(TOOLCHAIN_SOURCEKIT_SUPPORT_TRACING_H) */
