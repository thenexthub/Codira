//===--- Internal-XPC.h - ---------------------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKITD_INTERNAL_XPC_H
#define LLVM_SOURCEKITD_INTERNAL_XPC_H

#include "sourcekitd/Internal.h"

namespace sourcekitd {
namespace xpc {

static const char *KeyMsg = "msg";
static const char *KeyInternalMsg = "internal_msg";
static const char *KeySemaEditorDelay = "semantic_editor_delay";
static const char *KeyTracingEnabled = "tracing_enabled";
static const char *KeyMsgResponse = "response";
static const char *KeyCancelToken = "cancel_token";
static const char *KeyCancelRequest = "cancel_request";
static const char *KeyDisposeRequestHandle = "dispose_request_handle";
static const char *KeyPlugins = "plugins";

enum class Message {
  Initialization,
  Notification,
  UIDSynchronization,
};

} // namespace xpc
} // namespace sourcekitd

#endif
