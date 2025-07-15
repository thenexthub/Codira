//===--- CodiraCompile.cpp -------------------------------------------------===//
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

#include "CodiraEditorDiagConsumer.h"
#include "CodiraLangSupport.h"

#include "SourceKit/Support/FileSystemProvider.h"

#include "language/IDETool/CompileInstance.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/MemoryBuffer.h"

using namespace SourceKit;
using namespace language;

std::shared_ptr<compile::Session>
compile::SessionManager::getSession(StringRef name) {
  toolchain::sys::ScopedLock lock(mtx);
  auto i = sessions.find(name);
  if (i != sessions.end()) {
    return i->second;
  }

  bool inserted = false;
  std::tie(i, inserted) = sessions.try_emplace(
      name, std::make_shared<compile::Session>(CodiraExecutablePath,
                                               RuntimeResourcePath, Plugins));
  assert(inserted);
  return i->second;
}

void compile::SessionManager::clearSession(StringRef name) {
  toolchain::sys::ScopedLock lock(mtx);
  sessions.erase(name);
}

namespace {
class InvocationRequest final
    : public toolchain::TrailingObjects<InvocationRequest, char *, char> {
  friend class toolchain::TrailingObjects<InvocationRequest, char *, char>;
  size_t numArgs;

  size_t numTrailingObjects(OverloadToken<char *>) const { return numArgs; }

  MutableArrayRef<char *> getMutableArgs() {
    return {getTrailingObjects<char *>(), numArgs};
  }

  InvocationRequest(ArrayRef<const char *> Args) : numArgs(Args.size()) {
    // Copy the arguments to the buffer.
    char *ptr = getTrailingObjects<char>();
    auto thisArgs = getMutableArgs();
    size_t i = 0;
    for (const char *arg : Args) {
      thisArgs[i++] = ptr;
      auto size = strlen(arg) + 1;
      strncpy(ptr, arg, size);
      ptr += size;
    }
  }

public:
  static InvocationRequest *create(ArrayRef<const char *> Args) {
    size_t charBufSize = 0;
    for (auto arg : Args) {
      charBufSize += strlen(arg) + 1;
    }
    auto size = InvocationRequest::totalSizeToAlloc<char *, char>(Args.size(),
                                                                  charBufSize);
    auto *data = malloc(size);
    return new (data) InvocationRequest(Args);
  }

  ArrayRef<const char *> getArgs() const {
    return {getTrailingObjects<char *>(), numArgs};
  }
};
} // namespace

void compile::SessionManager::performCompileAsync(
    StringRef Name, ArrayRef<const char *> Args,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> fileSystem,
    std::shared_ptr<std::atomic<bool>> CancellationFlag,
    std::function<void(const RequestResult<CompilationResult> &)> Receiver) {
  auto session = getSession(Name);

  auto *request = InvocationRequest::create(Args);
  compileQueue.dispatch(
      [session, request, fileSystem, Receiver, CancellationFlag]() {
        LANGUAGE_DEFER {
          delete request;
        };

        // Cancelled during async dispatching?
        if (CancellationFlag->load(std::memory_order_relaxed)) {
          Receiver(RequestResult<CompilationResult>::cancelled());
          return;
        }

        EditorDiagConsumer diagC;
        auto stat =
            session->performCompile(request->getArgs(), fileSystem, &diagC,
                                    CancellationFlag);

        // Cancelled during the compilation?
        if (CancellationFlag->load(std::memory_order_relaxed)) {
          Receiver(RequestResult<CompilationResult>::cancelled());
          return;
        }

        SmallVector<DiagnosticEntryInfo, 0> diagEntryInfos;
        diagC.getAllDiagnostics(diagEntryInfos);
        Receiver(RequestResult<CompilationResult>::fromResult(
            {stat, diagEntryInfos}));
      },
      /*isStackDeep=*/true);
}

void CodiraLangSupport::performCompile(
    StringRef Name, ArrayRef<const char *> Args,
    std::optional<VFSOptions> vfsOptions,
    SourceKitCancellationToken CancellationToken,
    std::function<void(const RequestResult<CompilationResult> &)> Receiver) {

  std::string error;
  auto fileSystem =
      getFileSystem(vfsOptions, /*primaryFile=*/std::nullopt, error);
  if (!fileSystem) {
    Receiver(RequestResult<CompilationResult>::fromError(error));
    return;
  }

  std::shared_ptr<std::atomic<bool>> CancellationFlag = std::make_shared<std::atomic<bool>>(false);
  ReqTracker->setCancellationHandler(CancellationToken, [CancellationFlag]() {
    CancellationFlag->store(true, std::memory_order_relaxed);
  });

  CompileManager->performCompileAsync(Name, Args, std::move(fileSystem),
                                      CancellationFlag, Receiver);
}

void CodiraLangSupport::closeCompile(StringRef Name) {
  CompileManager->clearSession(Name);
}
