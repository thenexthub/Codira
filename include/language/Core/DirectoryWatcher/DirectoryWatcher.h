//===- DirectoryWatcher.h - Listens for directory file changes --*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_CORE_DIRECTORYWATCHER_DIRECTORYWATCHER_H
#define LANGUAGE_CORE_DIRECTORYWATCHER_DIRECTORYWATCHER_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Error.h"
#include <functional>
#include <memory>
#include <string>

namespace language::Core {
/// Provides notifications for file changes in a directory.
///
/// Invokes client-provided function on every filesystem event in the watched
/// directory. Initially the watched directory is scanned and for every file
/// found, an event is synthesized as if the file was added.
///
/// This is not a general purpose directory monitoring tool - list of
/// limitations follows.
///
/// Only flat directories with no subdirectories are supported. In case
/// subdirectories are present the behavior is unspecified - events *might* be
/// passed to Receiver on macOS (due to FSEvents being used) while they
/// *probably* won't be passed on Linux (due to inotify being used).
///
/// Known potential inconsistencies
/// - For files that are deleted befor the initial scan processed them, clients
/// might receive Removed notification without any prior Added notification.
/// - Multiple notifications might be produced when a file is added to the
/// watched directory during the initial scan. We are choosing the lesser evil
/// here as the only known alternative strategy would be to invalidate the
/// watcher instance and force user to create a new one whenever filesystem
/// event occurs during the initial scan but that would introduce continuous
/// restarting failure mode (watched directory is not always "owned" by the same
/// process that is consuming it). Since existing clients can handle duplicate
/// events well, we decided for simplicity.
///
/// Notifications are provided only for changes done through local user-space
/// filesystem interface. Specifically, it's unspecified if notification would
/// be provided in case of a:
/// - a file mmap-ed and changed
/// - a file changed via remote (NFS) or virtual (/proc) FS access to monitored
/// directory
/// - another filesystem mounted to the watched directory
///
/// No support for LLVM VFS.
///
/// It is unspecified whether notifications for files being deleted are sent in
/// case the whole watched directory is sent.
///
/// Directories containing "too many" files and/or receiving events "too
/// frequently" are not supported - if the initial scan can't be finished before
/// the watcher instance gets invalidated (see WatcherGotInvalidated) there's no
/// good error handling strategy - the only option for client is to destroy the
/// watcher, restart watching with new instance and hope it won't repeat.
class DirectoryWatcher {
public:
  struct Event {
    enum class EventKind {
      Removed,
      /// Content of a file was modified.
      Modified,
      /// The watched directory got deleted.
      WatchedDirRemoved,
      /// The DirectoryWatcher that originated this event is no longer valid and
      /// its behavior is unspecified.
      ///
      /// The prime case is kernel signalling to OS-specific implementation of
      /// DirectoryWatcher some resource limit being hit.
      /// *Usually* kernel starts dropping or squashing events together after
      /// that and so would DirectoryWatcher. This means that *some* events
      /// might still be passed to Receiver but this behavior is unspecified.
      ///
      /// Another case is after the watched directory itself is deleted.
      /// WatcherGotInvalidated will be received at least once during
      /// DirectoryWatcher instance lifetime - when handling errors this is done
      /// on best effort basis, when an instance is being destroyed then this is
      /// guaranteed.
      ///
      /// The only proper response to this kind of event is to destruct the
      /// originating DirectoryWatcher instance and create a new one.
      WatcherGotInvalidated
    };

    EventKind Kind;
    /// Filename that this event is related to or an empty string in
    /// case this event is related to the watched directory itself.
    std::string Filename;

    Event(EventKind Kind, toolchain::StringRef Filename)
        : Kind(Kind), Filename(Filename) {}
  };

  /// toolchain fatal_error if \param Path doesn't exist or isn't a directory.
  /// Returns toolchain::Expected Error if OS kernel API told us we can't start
  /// watching. In such case it's unclear whether just retrying has any chance
  /// to succeed.
  static toolchain::Expected<std::unique_ptr<DirectoryWatcher>>
  create(toolchain::StringRef Path,
         std::function<void(toolchain::ArrayRef<DirectoryWatcher::Event> Events,
                            bool IsInitial)>
             Receiver,
         bool WaitForInitialSync);

  virtual ~DirectoryWatcher() = default;
  DirectoryWatcher(const DirectoryWatcher &) = delete;
  DirectoryWatcher &operator=(const DirectoryWatcher &) = delete;
  DirectoryWatcher(DirectoryWatcher &&) = default;

protected:
  DirectoryWatcher() = default;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_DIRECTORYWATCHER_DIRECTORYWATCHER_H
