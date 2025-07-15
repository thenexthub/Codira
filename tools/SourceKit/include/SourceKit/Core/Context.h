//===--- Context.h - --------------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_CORE_CONTEXT_H
#define TOOLCHAIN_SOURCEKIT_CORE_CONTEXT_H

#include "SourceKit/Core/Toolchain.h"
#include "SourceKit/Support/CancellationToken.h"
#include "SourceKit/Support/Concurrency.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Mutex.h"
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace toolchain {
  class MemoryBuffer;
}

namespace SourceKit {
  class LangSupport;
  class NotificationCenter;
  class PluginSupport;

  class GlobalConfig {
  public:
    struct Settings {
      struct IDEInspectionOptions {

        /// Max count of reusing ASTContext for cached IDE inspection.
        unsigned MaxASTContextReuseCount = 100;

        /// Interval second for checking dependencies in cached IDE inspection.
        unsigned CheckDependencyInterval = 5;
      } IDEInspectionOpts;
    };

  private:
    Settings State;
    mutable toolchain::sys::Mutex Mtx;

  public:
    Settings
    update(std::optional<unsigned> IDEInspectionMaxASTContextReuseCount,
           std::optional<unsigned> IDEInspectionCheckDependencyInterval);
    Settings::IDEInspectionOptions getIDEInspectionOpts() const;
};

/// Keeps track of all requests that are currently in progress and coordinates
/// cancellation. All operations are no-ops if \c nullptr is used as a
/// cancellation token.
///
/// Tracked requests will be removed from this object when the request returns
/// a response (\c sourcekitd::handleRequest disposes the cancellation token).
/// We leak memory if a request is cancelled after it has returned a result
/// because we add an \c IsCancelled entry in that case but \c
/// sourcekitd::handleRequest won't have a chance to dispose of the token.
class RequestTracker {
  struct TrackedRequest {
    /// Whether the request has already been cancelled.
    bool IsCancelled = false;
    /// A handler that will be called as the request gets cancelled.
    std::function<void(void)> CancellationHandler;
  };

  /// Once we have information about a request (either a cancellation handler
  /// or information that it has been cancelled), it is added to this list.
  std::map<SourceKitCancellationToken, TrackedRequest> Requests;

  /// Guards the \c Requests variable.
  toolchain::sys::Mutex RequestsMtx;

  /// Must only be called if \c RequestsMtx has been claimed.
  /// Returns \c true if the request has already been cancelled. Requests that
  /// are not tracked are not assumed to be cancelled.
  bool isCancelledImpl(SourceKitCancellationToken CancellationToken) {
    if (Requests.count(CancellationToken) == 0) {
      return false;
    } else {
      return Requests[CancellationToken].IsCancelled;
    }
  }

public:
  /// Adds a \p CancellationHandler that will be called when the request
  /// associated with the \p CancellationToken is cancelled.
  /// If that request has already been cancelled when this method is called,
  /// \p CancellationHandler will be called synchronously.
  void setCancellationHandler(SourceKitCancellationToken CancellationToken,
                              std::function<void(void)> CancellationHandler) {
    if (!CancellationToken) {
      return;
    }
    toolchain::sys::ScopedLock L(RequestsMtx);
    if (isCancelledImpl(CancellationToken)) {
      if (CancellationHandler) {
        CancellationHandler();
      }
    } else {
      Requests[CancellationToken].CancellationHandler = CancellationHandler;
    }
  }

  /// Cancel the request with the given \p CancellationToken. If a cancellation
  /// handler is associated with it, call it.
  void cancel(SourceKitCancellationToken CancellationToken) {
    if (!CancellationToken) {
      return;
    }
    toolchain::sys::ScopedLock L(RequestsMtx);
    Requests[CancellationToken].IsCancelled = true;
    if (auto CancellationHandler =
            Requests[CancellationToken].CancellationHandler) {
      CancellationHandler();
    }
  }

  /// Stop tracking the request with the given \p CancellationToken, freeing up
  /// any memory needed for the tracking.
  void stopTracking(SourceKitCancellationToken CancellationToken) {
    if (!CancellationToken) {
      return;
    }
    toolchain::sys::ScopedLock L(RequestsMtx);
    Requests.erase(CancellationToken);
  }
};

class SlowRequestSimulator {
  std::shared_ptr<RequestTracker> ReqTracker;

public:
  SlowRequestSimulator(std::shared_ptr<RequestTracker> ReqTracker)
      : ReqTracker(ReqTracker) {}

  /// Simulate that a request takes \p DurationMs to execute. While waiting that
  /// duration, the request can be cancelled using the \p CancellationToken.
  /// Returns \c true if the request waited the required duration and \c false
  /// if it was cancelled.
  bool simulateLongRequest(int64_t DurationMs,
                           SourceKitCancellationToken CancellationToken) {
    auto Sema = Semaphore(0);
    ReqTracker->setCancellationHandler(CancellationToken,
                                       [&] { Sema.signal(); });
    bool DidTimeOut = Sema.wait(DurationMs);
    ReqTracker->setCancellationHandler(CancellationToken, nullptr);
    // If we timed out, we waited the required duration. If we didn't time out,
    // the semaphore was cancelled.
    return DidTimeOut;
  }
};

class Context {
  /// The path of the language-frontend executable.
  /// Used to find clang relative to it.
  std::string CodiraExecutablePath;
  std::string RuntimeLibPath;
  std::unique_ptr<LangSupport> CodiraLang;
  std::shared_ptr<NotificationCenter> NotificationCtr;
  std::shared_ptr<GlobalConfig> Config;
  std::shared_ptr<RequestTracker> ReqTracker;
  std::shared_ptr<PluginSupport> Plugins;
  std::shared_ptr<SlowRequestSimulator> SlowRequestSim;

public:
  Context(StringRef CodiraExecutablePath, StringRef RuntimeLibPath,
          toolchain::function_ref<std::unique_ptr<LangSupport>(Context &)>
              LangSupportFactoryFn,
          toolchain::function_ref<std::shared_ptr<PluginSupport>(Context &)>
              PluginSupportFactoryFn,
          bool shouldDispatchNotificationsOnMain = true);
  ~Context();

  StringRef getCodiraExecutablePath() const { return CodiraExecutablePath; }

  StringRef getRuntimeLibPath() const { return RuntimeLibPath; }

  LangSupport &getCodiraLangSupport() { return *CodiraLang; }

  std::shared_ptr<NotificationCenter> getNotificationCenter() { return NotificationCtr; }

  std::shared_ptr<GlobalConfig> getGlobalConfiguration() { return Config; }

  std::shared_ptr<PluginSupport> getPlugins() { return Plugins; }

  std::shared_ptr<SlowRequestSimulator> getSlowRequestSimulator() {
    return SlowRequestSim;
  }

  std::shared_ptr<RequestTracker> getRequestTracker() { return ReqTracker; }
};

} // namespace SourceKit

#endif
