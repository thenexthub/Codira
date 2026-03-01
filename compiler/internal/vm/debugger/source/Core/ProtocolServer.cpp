//===-- ProtocolServer.cpp ------------------------------------------------===//
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

#include "lldb/Core/ProtocolServer.h"
#include "lldb/Core/PluginManager.h"
#include "llvm/Support/Error.h"

using namespace lldb_private;
using namespace lldb;

static std::pair<llvm::StringMap<ProtocolServerUP> &, std::mutex &> Servers() {
  static llvm::StringMap<ProtocolServerUP> g_protocol_server_instances;
  static std::mutex g_mutex;
  return {g_protocol_server_instances, g_mutex};
}

ProtocolServer *ProtocolServer::GetOrCreate(llvm::StringRef name) {
  auto [protocol_server_instances, mutex] = Servers();

  std::lock_guard<std::mutex> guard(mutex);

  auto it = protocol_server_instances.find(name);
  if (it != protocol_server_instances.end())
    return it->second.get();

  if (ProtocolServerCreateInstance create_callback =
          PluginManager::GetProtocolCreateCallbackForPluginName(name)) {
    auto pair = protocol_server_instances.try_emplace(name, create_callback());
    return pair.first->second.get();
  }

  return nullptr;
}

std::vector<llvm::StringRef> ProtocolServer::GetSupportedProtocols() {
  std::vector<llvm::StringRef> supported_protocols;
  size_t i = 0;

  for (llvm::StringRef protocol_name =
           PluginManager::GetProtocolServerPluginNameAtIndex(i++);
       !protocol_name.empty();
       protocol_name = PluginManager::GetProtocolServerPluginNameAtIndex(i++)) {
    supported_protocols.push_back(protocol_name);
  }

  return supported_protocols;
}

llvm::Error ProtocolServer::Terminate() {
  llvm::Error error = llvm::Error::success();

  auto [protocol_server_instances, mutex] = Servers();
  std::lock_guard<std::mutex> guard(mutex);
  for (auto &instance : protocol_server_instances) {
    if (llvm::Error instance_error = instance.second->Stop())
      error = llvm::joinErrors(std::move(error), std::move(instance_error));
  }

  protocol_server_instances.clear();

  return error;
}
