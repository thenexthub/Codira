//===--- CachingUtils.h -----------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_CACHINGUTILS_H
#define LANGUAGE_FRONTEND_CACHINGUTILS_H

#include "language/Frontend/CASOutputBackends.h"
#include "language/Frontend/CachedDiagnostics.h"
#include "language/Frontend/FrontendInputsAndOutputs.h"
#include "language/Frontend/FrontendOptions.h"
#include "clang/CAS/CASOptions.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/CAS/ActionCache.h"
#include "toolchain/CAS/CASReference.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/PrefixMapper.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/VirtualOutputBackend.h"
#include <memory>

namespace language {

class DiagnosticHelper;

/// Create a language caching output backend that stores the output from
/// compiler into a CAS.
toolchain::IntrusiveRefCntPtr<cas::CodiraCASOutputBackend>
createCodiraCachingOutputBackend(
    toolchain::cas::ObjectStore &CAS, toolchain::cas::ActionCache &Cache,
    toolchain::cas::ObjectRef BaseKey,
    const FrontendInputsAndOutputs &InputsAndOutputs,
    const FrontendOptions &Opts, FrontendOptions::ActionType Action);

/// Replay the output of the compilation from cache.
/// Return true if outputs are replayed, false otherwise.
bool replayCachedCompilerOutputs(toolchain::cas::ObjectStore &CAS,
                                 toolchain::cas::ActionCache &Cache,
                                 toolchain::cas::ObjectRef BaseKey,
                                 DiagnosticEngine &Diag,
                                 const FrontendOptions &Opts,
                                 CachingDiagnosticsProcessor &CDP,
                                 bool CacheRemarks, bool UseCASBackend);

/// Replay the output of the compilation from cache for one input file.
/// Return true if outputs are replayed, false otherwise.
bool replayCachedCompilerOutputsForInput(toolchain::cas::ObjectStore &CAS,
                                         toolchain::cas::ObjectRef OutputRef,
                                         const InputFile &Input,
                                         unsigned InputIndex,
                                         DiagnosticEngine &Diag,
                                         DiagnosticHelper &DiagHelper,
                                         toolchain::vfs::OutputBackend &OutBackend,
                                         const FrontendOptions &Opts,
                                         CachingDiagnosticsProcessor &CDP,
                                         bool CacheRemarks, bool UseCASBackend);

/// Load the cached compile result from cache.
std::unique_ptr<toolchain::MemoryBuffer> loadCachedCompileResultFromCacheKey(
    toolchain::cas::ObjectStore &CAS, toolchain::cas::ActionCache &Cache,
    DiagnosticEngine &Diag, toolchain::StringRef CacheKey, file_types::ID Type,
    toolchain::StringRef Filename = "");

toolchain::Expected<toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>>
createCASFileSystem(toolchain::cas::ObjectStore &CAS,
                    const std::string &IncludeTreeRoot,
                    const std::string &IncludeTreeFileList);

std::vector<std::string> remapPathsFromCommandLine(
    ArrayRef<std::string> Args,
    toolchain::function_ref<std::string(StringRef)> RemapCallback);

namespace cas {
class CachedResultLoader {
public:
  CachedResultLoader(toolchain::cas::ObjectStore &CAS,
                     toolchain::cas::ObjectRef OutputRef)
      : CAS(CAS), OutputRef(OutputRef) {}

  using CallbackTy =
      toolchain::function_ref<toolchain::Error(file_types::ID, toolchain::cas::ObjectRef)>;

  /// Replay the cached result.
  toolchain::Error replay(CallbackTy Callback);

private:
  toolchain::cas::ObjectStore &CAS;
  toolchain::cas::ObjectRef OutputRef;
};
} // end namespace cas
} // end namespace language

#endif
