//===-- ProtocolEvents.cpp ------------------------------------------------===//
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

#include "Protocol/ProtocolEvents.h"
#include "JSONUtils.h"
#include "llvm/Support/JSON.h"

using namespace llvm;

namespace lldb_dap::protocol {

json::Value toJSON(const CapabilitiesEventBody &CEB) {
  return json::Object{{"capabilities", CEB.capabilities}};
}

json::Value toJSON(const ModuleEventBody::Reason &MEBR) {
  switch (MEBR) {
  case ModuleEventBody::eReasonNew:
    return "new";
  case ModuleEventBody::eReasonChanged:
    return "changed";
  case ModuleEventBody::eReasonRemoved:
    return "removed";
  }
  llvm_unreachable("unhandled module event reason!.");
}

json::Value toJSON(const ModuleEventBody &MEB) {
  return json::Object{{"reason", MEB.reason}, {"module", MEB.module}};
}

llvm::json::Value toJSON(const InvalidatedEventBody::Area &IEBA) {
  switch (IEBA) {
  case InvalidatedEventBody::eAreaAll:
    return "all";
  case InvalidatedEventBody::eAreaStacks:
    return "stacks";
  case InvalidatedEventBody::eAreaThreads:
    return "threads";
  case InvalidatedEventBody::eAreaVariables:
    return "variables";
  }
  llvm_unreachable("unhandled invalidated event area!.");
}

llvm::json::Value toJSON(const InvalidatedEventBody &IEB) {
  json::Object Result{{"areas", IEB.areas}};
  if (IEB.threadId)
    Result.insert({"threadId", IEB.threadId});
  if (IEB.stackFrameId)
    Result.insert({"stackFrameId", IEB.stackFrameId});
  return Result;
}

llvm::json::Value toJSON(const MemoryEventBody &MEB) {
  return json::Object{
      {"memoryReference", EncodeMemoryReference(MEB.memoryReference)},
      {"offset", MEB.offset},
      {"count", MEB.count}};
}

} // namespace lldb_dap::protocol
