//===--- NotificationCenter.h - ---------------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKIT_CORE_NOTIFICATIONCENTER_H
#define LLVM_SOURCEKIT_CORE_NOTIFICATIONCENTER_H

#include "SourceKit/Core/LLVM.h"
#include "SourceKit/Support/Tracing.h"
#include "SourceKit/Support/UIdent.h"
#include "llvm/Support/Mutex.h"
#include <functional>
#include <vector>

namespace SourceKit {

struct DiagnosticEntryInfo;

typedef std::function<void(StringRef DocumentName)>
    DocumentUpdateNotificationReceiver;

typedef std::function<void(uint64_t CompileID, trace::OperationKind,
                           const trace::SwiftInvocation &)>
    CompileWillStartNotificationReceiver;
typedef std::function<void(uint64_t CompileID, trace::OperationKind,
                           ArrayRef<DiagnosticEntryInfo>)>
    CompileDidFinishNotificationReceiver;

class NotificationCenter {
  bool DispatchToMain;
  std::vector<DocumentUpdateNotificationReceiver> DocUpdReceivers;
  std::vector<std::function<void(void)>> TestReceivers;
  std::vector<std::function<void(void)>> SemaEnabledReceivers;
  std::vector<CompileWillStartNotificationReceiver> CompileWillStartReceivers;
  std::vector<CompileDidFinishNotificationReceiver> CompileDidFinishReceivers;
  mutable llvm::sys::Mutex Mtx;

public:
  explicit NotificationCenter(bool dispatchToMain);
  ~NotificationCenter();

  void addDocumentUpdateNotificationReceiver(
      DocumentUpdateNotificationReceiver Receiver);
  void addTestNotificationReceiver(std::function<void(void)> Receiver);
  void addSemaEnabledNotificationReceiver(std::function<void(void)> Receiver);
  void addCompileWillStartNotificationReceiver(
      CompileWillStartNotificationReceiver Receiver);
  void addCompileDidFinishNotificationReceiver(
      CompileDidFinishNotificationReceiver Receiver);

  void postDocumentUpdateNotification(StringRef DocumentName) const;
  void postTestNotification() const;
  void postSemaEnabledNotification() const;
  void
  postCompileWillStartNotification(uint64_t CompileID,
                                   trace::OperationKind OpKind,
                                   const trace::SwiftInvocation &Inv) const;
  void postCompileDidFinishNotification(
      uint64_t CompileID, trace::OperationKind OpKind,
      ArrayRef<DiagnosticEntryInfo> Diagnostics) const;
};

} // namespace SourceKit

#endif
