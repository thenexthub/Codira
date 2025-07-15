//===--- NotificationCenter.cpp -------------------------------------------===//
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

#include "SourceKit/Core/NotificationCenter.h"
#include "SourceKit/Core/LangSupport.h"
#include "SourceKit/Support/Concurrency.h"

using namespace SourceKit;

NotificationCenter::NotificationCenter(bool dispatchToMain)
  : DispatchToMain(dispatchToMain) {
}
NotificationCenter::~NotificationCenter() {}

void NotificationCenter::addDocumentUpdateNotificationReceiver(
    DocumentUpdateNotificationReceiver Receiver) {

  toolchain::sys::ScopedLock L(Mtx);
  DocUpdReceivers.push_back(Receiver);
}

void NotificationCenter::addTestNotificationReceiver(
    std::function<void(void)> Receiver) {
  toolchain::sys::ScopedLock L(Mtx);
  TestReceivers.push_back(std::move(Receiver));
}
void NotificationCenter::addSemaEnabledNotificationReceiver(
    std::function<void(void)> Receiver) {
  toolchain::sys::ScopedLock L(Mtx);
  SemaEnabledReceivers.push_back(std::move(Receiver));
}
void NotificationCenter::addCompileWillStartNotificationReceiver(
    CompileWillStartNotificationReceiver Receiver) {
  toolchain::sys::ScopedLock L(Mtx);
  CompileWillStartReceivers.push_back(std::move(Receiver));
}
void NotificationCenter::addCompileDidFinishNotificationReceiver(
    CompileDidFinishNotificationReceiver Receiver) {
  toolchain::sys::ScopedLock L(Mtx);
  CompileDidFinishReceivers.push_back(std::move(Receiver));
}

#define POST_NOTIFICATION(Receivers, Args...)                                  \
  do {                                                                         \
    decltype(Receivers) recvs;                                                 \
    {                                                                          \
      toolchain::sys::ScopedLock L(Mtx);                                            \
      recvs = Receivers;                                                       \
    }                                                                          \
    auto sendNote = [=] {                                                      \
      for (auto &Fn : recvs)                                                   \
        Fn(Args);                                                              \
    };                                                                         \
    if (DispatchToMain)                                                        \
      WorkQueue::dispatchOnMain(sendNote);                                     \
    else                                                                       \
      sendNote();                                                              \
  } while (0)

void NotificationCenter::postDocumentUpdateNotification(
    StringRef DocumentName) const {
  std::string docName = DocumentName.str();
  POST_NOTIFICATION(DocUpdReceivers, docName);
}
void NotificationCenter::postTestNotification() const {
  POST_NOTIFICATION(TestReceivers, );
}
void NotificationCenter::postSemaEnabledNotification() const {
  POST_NOTIFICATION(SemaEnabledReceivers, );
}
void NotificationCenter::postCompileWillStartNotification(
    uint64_t CompileID, trace::OperationKind OpKind,
    const trace::CodiraInvocation &Inv) const {
  trace::CodiraInvocation inv(Inv);
  POST_NOTIFICATION(CompileWillStartReceivers, CompileID, OpKind, inv);
}
void NotificationCenter::postCompileDidFinishNotification(
    uint64_t CompileID, trace::OperationKind OpKind,
    ArrayRef<DiagnosticEntryInfo> Diagnostics) const {
  std::vector<DiagnosticEntryInfo> diags(Diagnostics);
  POST_NOTIFICATION(CompileDidFinishReceivers, CompileID, OpKind, diags);
}
