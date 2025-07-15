//===--- Bridging/PluginBridging.cpp --------------------------------------===//
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

#include "language/AST/ASTBridging.h"

#include "language/AST/PluginRegistry.h"

using namespace language;

//===----------------------------------------------------------------------===//
// MARK: Plugin
//===----------------------------------------------------------------------===//

PluginCapabilityPtr Plugin_getCapability(PluginHandle handle) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  return plugin->getCapability();
}

void Plugin_setCapability(PluginHandle handle, PluginCapabilityPtr data) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  plugin->setCapability(data);
}

void Plugin_lock(PluginHandle handle) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  plugin->lock();
}

void Plugin_unlock(PluginHandle handle) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  plugin->unlock();
}

bool Plugin_spawnIfNeeded(PluginHandle handle) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  auto error = plugin->spawnIfNeeded();
  bool hadError(error);
  toolchain::consumeError(std::move(error));
  return hadError;
}

bool Plugin_sendMessage(PluginHandle handle, const BridgedData data) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  StringRef message(data.BaseAddress, data.Length);
  auto error = plugin->sendMessage(message);
  if (error) {
    // FIXME: Pass the error message back to the caller.
    toolchain::consumeError(std::move(error));
    //    toolchain::handleAllErrors(std::move(error), [](const toolchain::ErrorInfoBase
    //    &err) {
    //      toolchain::errs() << err.message() << "\n";
    //    });
    return true;
  }
  return false;
}

bool Plugin_waitForNextMessage(PluginHandle handle, BridgedData *out) {
  auto *plugin = static_cast<CompilerPlugin *>(handle);
  auto result = plugin->waitForNextMessage();
  if (!result) {
    // FIXME: Pass the error message back to the caller.
    toolchain::consumeError(result.takeError());
    //    toolchain::handleAllErrors(result.takeError(), [](const toolchain::ErrorInfoBase
    //    &err) {
    //      toolchain::errs() << err.message() << "\n";
    //    });
    return true;
  }
  auto &message = result.get();
  auto size = message.size();
  auto outPtr = malloc(size);
  memcpy(outPtr, message.data(), size);
  *out = BridgedData{(const char *)outPtr, size};
  return false;
}
