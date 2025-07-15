//===------CASOutputBackends.h-- ---------------------------------*-C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_CASOUTPUTBACKENDS_H
#define LANGUAGE_FRONTEND_CASOUTPUTBACKENDS_H

#include "language/Frontend/FrontendInputsAndOutputs.h"
#include "language/Frontend/FrontendOptions.h"
#include "toolchain/CAS/ActionCache.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/VirtualOutputBackends.h"
#include "toolchain/Support/VirtualOutputFile.h"

namespace language::cas {

class CodiraCASOutputBackend : public toolchain::vfs::OutputBackend {
  void anchor() override;

protected:
  toolchain::IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override;

  toolchain::Expected<std::unique_ptr<toolchain::vfs::OutputFileImpl>>
  createFileImpl(toolchain::StringRef ResolvedPath,
                 std::optional<toolchain::vfs::OutputConfig> Config) override;

  virtual toolchain::Error storeImpl(toolchain::StringRef Path, toolchain::StringRef Bytes,
                                unsigned InputIndex, file_types::ID OutputKind);

private:
  file_types::ID getOutputFileType(toolchain::StringRef Path) const;

  /// Return true if the file type is stored into CAS Backend directly.
  static bool isStoredDirectly(file_types::ID Kind);

public:
  CodiraCASOutputBackend(toolchain::cas::ObjectStore &CAS,
                        toolchain::cas::ActionCache &Cache,
                        toolchain::cas::ObjectRef BaseKey,
                        const FrontendInputsAndOutputs &InputsAndOutputs,
                        const FrontendOptions &Opts,
                        FrontendOptions::ActionType Action);
  ~CodiraCASOutputBackend();

  toolchain::Error storeCachedDiagnostics(unsigned InputIndex,
                                     toolchain::StringRef Bytes);

  toolchain::Error storeMakeDependenciesFile(StringRef OutputFilename,
                                        toolchain::StringRef Bytes);

  /// Store the MCCAS CASID \p ID as the object file output for the input
  /// that corresponds to the \p OutputFilename
  toolchain::Error storeMCCASObjectID(StringRef OutputFilename, toolchain::cas::CASID ID);

private:
  class Implementation;
  Implementation &Impl;
};

} // end namespace language::cas

#endif
