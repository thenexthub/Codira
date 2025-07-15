//===--- CompileInstance.h ------------------------------------------------===//
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

#ifndef LANGUAGE_IDE_COMPILEINSTANCE_H
#define LANGUAGE_IDE_COMPILEINSTANCE_H

#include "language/Frontend/Frontend.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Chrono.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/VirtualFileSystem.h"

namespace language {

class CompilerInstance;
class DiagnosticConsumer;
class PluginRegistry;

namespace ide {

/// Manages \c CompilerInstance for completion like operations.
class CompileInstance {
  const std::string &CodiraExecutablePath;
  const std::string &RuntimeResourcePath;
  const std::shared_ptr<language::PluginRegistry> Plugins;

  struct Options {
    unsigned MaxASTReuseCount = 100;
    unsigned DependencyCheckIntervalSecond = 5;
  } Opts;

  std::mutex mtx;

  std::unique_ptr<CompilerInstance> CI;
  toolchain::hash_code CachedArgHash;
  std::atomic<bool> CachedCIInvalidated;
  toolchain::sys::TimePoint<> DependencyCheckedTimestamp;
  toolchain::StringMap<toolchain::hash_code> InMemoryDependencyHash;
  unsigned CachedReuseCount;

  bool shouldCheckDependencies() const;

  /// Perform cached sema. Returns \c true if the CI is not reusable.
  bool performCachedSemaIfPossible(DiagnosticConsumer *DiagC);

  /// Setup the CI with \p Args . Returns \c true if failed.
  bool setupCI(toolchain::ArrayRef<const char *> Args,
                 toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
                 DiagnosticConsumer *DiagC);

  /// Perform Parse and Sema, potentially CI from previous compilation is
  /// reused. Returns \c true if there was any error.
  bool performSema(toolchain::ArrayRef<const char *> Args,
                   toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
                   DiagnosticConsumer *DiagC,
                   std::shared_ptr<std::atomic<bool>> CancellationFlag);

public:
  CompileInstance(const std::string &CodiraExecutablePath,
                  const std::string &RuntimeResourcePath,
                  std::shared_ptr<language::PluginRegistry> Plugins = nullptr)
      : CodiraExecutablePath(CodiraExecutablePath),
        RuntimeResourcePath(RuntimeResourcePath), Plugins(Plugins),
        CachedCIInvalidated(false), CachedReuseCount(0) {}

  /// NOTE: \p Args is only used for checking the equaity of the invocation.
  /// Since this function assumes that it is already normalized, exact the same
  /// arguments including their order is considered as the same invocation.
  bool
  performCompile(toolchain::ArrayRef<const char *> Args,
                 toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
                 DiagnosticConsumer *DiagC,
                 std::shared_ptr<std::atomic<bool>> CancellationFlag);
};

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_COMPILEINSTANCE_H
