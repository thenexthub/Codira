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

#ifndef SWIFT_FRONTEND_CASOUTPUTBACKENDS_H
#define SWIFT_FRONTEND_CASOUTPUTBACKENDS_H

#include "language/Frontend/FrontendInputsAndOutputs.h"
#include "language/Frontend/FrontendOptions.h"
#include "llvm/CAS/ActionCache.h"
#include "llvm/CAS/ObjectStore.h"
#include "llvm/Support/VirtualOutputBackends.h"
#include "llvm/Support/VirtualOutputFile.h"

namespace language::cas {

class SwiftCASOutputBackend : public llvm::vfs::OutputBackend {
  void anchor() override;

protected:
  llvm::IntrusiveRefCntPtr<OutputBackend> cloneImpl() const override;

  llvm::Expected<std::unique_ptr<llvm::vfs::OutputFileImpl>>
  createFileImpl(llvm::StringRef ResolvedPath,
                 std::optional<llvm::vfs::OutputConfig> Config) override;

  virtual llvm::Error storeImpl(llvm::StringRef Path, llvm::StringRef Bytes,
                                unsigned InputIndex, file_types::ID OutputKind);

private:
  file_types::ID getOutputFileType(llvm::StringRef Path) const;

  /// Return true if the file type is stored into CAS Backend directly.
  static bool isStoredDirectly(file_types::ID Kind);

public:
  SwiftCASOutputBackend(llvm::cas::ObjectStore &CAS,
                        llvm::cas::ActionCache &Cache,
                        llvm::cas::ObjectRef BaseKey,
                        const FrontendInputsAndOutputs &InputsAndOutputs,
                        const FrontendOptions &Opts,
                        FrontendOptions::ActionType Action);
  ~SwiftCASOutputBackend();

  llvm::Error storeCachedDiagnostics(unsigned InputIndex,
                                     llvm::StringRef Bytes);

  llvm::Error storeMakeDependenciesFile(StringRef OutputFilename,
                                        llvm::StringRef Bytes);

  /// Store the MCCAS CASID \p ID as the object file output for the input
  /// that corresponds to the \p OutputFilename
  llvm::Error storeMCCASObjectID(StringRef OutputFilename, llvm::cas::CASID ID);

private:
  class Implementation;
  Implementation &Impl;
};

} // end namespace language::cas

#endif
