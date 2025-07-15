//===--- PluginRegistry.cpp -----------------------------------------------===//
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

#include "language/AST/PluginRegistry.h"

#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/LoadDynamicLibrary.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/Program.h"
#include "language/Basic/Sandbox.h"
#include "language/Basic/StringExtras.h"
#include "toolchain/Support/Endian.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Config/config.h"

#include <signal.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif

using namespace language;

PluginRegistry::PluginRegistry() {
  dumpMessaging = ::getenv("LANGUAGE_DUMP_PLUGIN_MESSAGING") != nullptr;
}

CompilerPlugin::~CompilerPlugin() {
  // Let ASTGen do cleanup things.
  if (this->cleanup)
    this->cleanup();
}

toolchain::Expected<std::unique_ptr<InProcessPlugins>>
InProcessPlugins::create(const char *serverPath) {
  std::string err;
  auto server = loadLibrary(serverPath, &err);
  if (!server) {
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(), err);
  }

  auto funcPtr =
      getAddressOfSymbol(server, "language_inproc_plugins_handle_message");
  if (!funcPtr) {
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "entry point not found in '%s'", serverPath);
  }
  return std::unique_ptr<InProcessPlugins>(new InProcessPlugins(
      serverPath, reinterpret_cast<HandleMessageFunction>(funcPtr)));
}

toolchain::Error InProcessPlugins::sendMessage(toolchain::StringRef message) {
  assert(receivedResponse.empty() &&
         "sendMessage() called before consuming previous response?");

  if (shouldDumpMessaging()) {
    toolchain::dbgs() << "->(plugin:0) " << message << '\n';
  }

  char *responseData = nullptr;
  size_t responseLength = 0;
  bool hadError = handleMessageFn(message.data(), message.size(), &responseData,
                                  &responseLength);

  // 'responseData' now holds a response message or error message depending on
  // 'hadError'. Either way, it's our responsibility to deallocate it.
  LANGUAGE_DEFER { free(responseData); };
  if (hadError) {
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   StringRef(responseData, responseLength));
  }

  // Store the response and wait for 'waitForNextMessage()' call.
  receivedResponse = std::string(responseData, responseLength);

  if (shouldDumpMessaging()) {
    toolchain::dbgs() << "<-(plugin:0) " << receivedResponse << "\n";
  }

  assert(!receivedResponse.empty() && "received empty response");
  return toolchain::Error::success();
}

toolchain::Expected<std::string> InProcessPlugins::waitForNextMessage() {
  assert(!receivedResponse.empty() &&
         "waitForNextMessage() called without response data.");
  LANGUAGE_DEFER { receivedResponse = ""; };
  return std::move(receivedResponse);
}

toolchain::Expected<CompilerPlugin *>
PluginRegistry::getInProcessPlugins(toolchain::StringRef serverPath) {
  std::lock_guard<std::mutex> lock(mtx);
  if (!inProcessPlugins) {
    auto server = InProcessPlugins::create(serverPath.str().c_str());
    if (!server) {
      return toolchain::handleErrors(
          server.takeError(), [&](const toolchain::ErrorInfoBase &err) {
            return toolchain::createStringError(
                err.convertToErrorCode(),
                "failed to load in-process plugin server: " + serverPath +
                    "; " + err.message());
          });
    }
    inProcessPlugins = std::move(server.get());
  } else if (inProcessPlugins->getPath() != serverPath) {
    return toolchain::createStringError(
        toolchain::inconvertibleErrorCode(),
        "loading multiple in-process servers are not supported: '%s' and '%s'",
        inProcessPlugins->getPath().data(), serverPath.str().c_str());
  }
  inProcessPlugins->setDumpMessaging(dumpMessaging);

  return inProcessPlugins.get();
}

toolchain::Expected<CompilerPlugin *>
PluginRegistry::loadExecutablePlugin(StringRef path, bool disableSandbox) {
  toolchain::sys::fs::file_status stat;
  if (auto err = toolchain::sys::fs::status(path, stat)) {
    return toolchain::errorCodeToError(err);
  }

  std::lock_guard<std::mutex> lock(mtx);

  // See if the plugin is already loaded.
  auto &storage = LoadedPluginExecutables[path];
  if (storage) {
    // See if the loaded one is still usable.
    if (storage->getLastModificationTime() == stat.getLastModificationTime())
      return storage.get();

    // The plugin is updated. Close the previously opened plugin.
    storage = nullptr;
  }

  if (!toolchain::sys::fs::exists(stat)) {
    return toolchain::createStringError(std::errc::no_such_file_or_directory,
                                   "not found");
  }

  if (!toolchain::sys::fs::can_execute(path)) {
    return toolchain::createStringError(std::errc::permission_denied,
                                   "not executable");
  }

  auto plugin = std::make_unique<LoadedExecutablePlugin>(
      path, stat.getLastModificationTime(), disableSandbox);

  plugin->setDumpMessaging(dumpMessaging);

  // Launch here to see if it's actually executable, and diagnose (by returning
  // an error) if necessary.
  if (auto error = plugin->spawnIfNeeded()) {
    return std::move(error);
  }

  storage = std::move(plugin);
  return storage.get();
}

toolchain::Error LoadedExecutablePlugin::spawnIfNeeded() {
  if (Process) {
    // NOTE: We don't check the mtime here because 'stat(2)' call is too heavy.
    // PluginRegistry::loadExecutablePlugin() checks it and replace this object
    // itself if the plugin is updated.
    return toolchain::Error::success();
  }

  // Create command line arguments.
  SmallVector<StringRef, 4> command{getPath()};

  // Apply sandboxing.
  toolchain::BumpPtrAllocator Allocator;
  if (!disableSandbox) {
    Sandbox::apply(command, Allocator);
  }

  // Launch.
  auto childInfo = ExecuteWithPipe(command[0], command);
  if (!childInfo) {
    return toolchain::errorCodeToError(childInfo.getError());
  }

  Process = std::make_unique<PluginProcess>(childInfo->ProcessInfo,
                                            childInfo->Read, childInfo->Write);

  // Call "on reconnect" callbacks.
  for (auto *callback : getOnReconnectCallbacks()) {
    (*callback)();
  }

  return toolchain::Error::success();
}

LoadedExecutablePlugin::PluginProcess::~PluginProcess() {
#if defined(_WIN32)
  _close(input);
  _close(output);
#else
  close(input);
  close(output);
  kill(process.Pid, SIGTERM);
#endif

  // Set `SecondsToWait` non-zero so it waits for the timeout and kill it after
  // that. Usually when the pipe is closed above, the plugin detects the EOF in
  // the stdin and exits immediately, so this usually doesn't wait for the
  // timeout. Note that we can't use '0' because it performs a non-blocking
  // wait, which make the plugin a zombie if it hasn't exited.
  toolchain::sys::Wait(process, /*SecondsToWait=*/1);
}

ssize_t LoadedExecutablePlugin::PluginProcess::read(void *buf,
                                                    size_t nbyte) const {
#if defined(_WIN32)
  size_t nread = 0;
  while (nread < nbyte) {
    int n = _read(input, static_cast<char*>(buf) + nread,
                  std::min(static_cast<size_t>(UINT32_MAX), nbyte - nread));
    if (n <= 0)
      break;
    nread += n;
  }
  return nread;
#else
  ssize_t bytesToRead = nbyte;
  void *ptr = buf;

#if defined(SIGPIPE)
  /// Ignore SIGPIPE while reading.
  auto *old_handler = signal(SIGPIPE, SIG_IGN);
  LANGUAGE_DEFER { signal(SIGPIPE, old_handler); };
#endif

  while (bytesToRead > 0) {
    ssize_t readingSize = std::min(ssize_t(INT32_MAX), bytesToRead);
    ssize_t readSize = ::read(input, ptr, readingSize);
    if (readSize <= 0) {
      // 0: EOF (the plugin exited?), -1: error (e.g. broken pipe.)
      // FIXME: Mark the plugin 'stale' and relaunch later.
      break;
    }
    ptr = static_cast<char *>(ptr) + readSize;
    bytesToRead -= readSize;
  }

  return nbyte - bytesToRead;
#endif
}

ssize_t LoadedExecutablePlugin::PluginProcess::write(const void *buf,
                                                     size_t nbyte) const {
#if defined(_WIN32)
  size_t nwritten = 0;
  while (nwritten < nbyte) {
    int n = _write(output, static_cast<const char *>(buf) + nwritten,
                   std::min(static_cast<size_t>(UINT32_MAX), nbyte - nwritten));
    if (n <= 0)
      break;
    nwritten += n;
  }
  return nwritten;
#else
  ssize_t bytesToWrite = nbyte;
  const void *ptr = buf;

#if defined(SIGPIPE)
  /// Ignore SIGPIPE while writing.
  auto *old_handler = signal(SIGPIPE, SIG_IGN);
  LANGUAGE_DEFER { signal(SIGPIPE, old_handler); };
#endif

  while (bytesToWrite > 0) {
    ssize_t writingSize = std::min(ssize_t(INT32_MAX), bytesToWrite);
    ssize_t writtenSize = ::write(output, ptr, writingSize);
    if (writtenSize <= 0) {
      // -1: error (e.g. broken pipe,)
      // FIXME: Mark the plugin 'stale' and relaunch later.
      break;
    }
    ptr = static_cast<const char *>(ptr) + writtenSize;
    bytesToWrite -= writtenSize;
  }
  return nbyte - bytesToWrite;
#endif
}

toolchain::Error LoadedExecutablePlugin::sendMessage(toolchain::StringRef message) {
  ssize_t writtenSize = 0;

  if (shouldDumpMessaging()) {
    toolchain::dbgs() << "->(plugin:" << Process->process.Pid << ") " << message << '\n';
  }

  const char *data = message.data();
  size_t size = message.size();

  // Write header (message size).
  uint64_t header = toolchain::support::endian::byte_swap(uint64_t(size),
                                                     toolchain::endianness::little);
  writtenSize = Process->write(&header, sizeof(header));
  if (writtenSize != sizeof(header)) {
    setStale();
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "failed to write plugin message header");
  }

  // Write message.
  writtenSize = Process->write(data, size);
  if (writtenSize != ssize_t(size)) {
    setStale();
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "failed to write plugin message data");
  }

  return toolchain::Error::success();
}

toolchain::Expected<std::string> LoadedExecutablePlugin::waitForNextMessage() {
  ssize_t readSize = 0;

  // Read header (message size).
  uint64_t header;
  readSize = Process->read(&header, sizeof(header));

  if (readSize != sizeof(header)) {
    setStale();
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "failed to read plugin message header");
  }

  size_t size =
      toolchain::support::endian::read<uint64_t>(&header, toolchain::endianness::little);

  // Read message.
  std::string message;
  message.reserve(size);
  auto sizeToRead = size;
  while (sizeToRead > 0) {
    char buffer[4096];
    readSize = Process->read(buffer, std::min(sizeof(buffer), sizeToRead));
    if (readSize == 0) {
      setStale();
      return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                     "failed to read plugin message data");
    }
    sizeToRead -= readSize;
    message.append(buffer, readSize);
  }

  if (shouldDumpMessaging()) {
    toolchain::dbgs() << "<-(plugin:" << Process->process.Pid << ") " << message << "\n";
  }

  return message;
}
