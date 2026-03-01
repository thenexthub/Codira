//===- DWARFLinkerGlobalData.h ----------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_DWARFLINKER_PARALLEL_DWARFLINKERGLOBALDATA_H
#define LLVM_LIB_DWARFLINKER_PARALLEL_DWARFLINKERGLOBALDATA_H

#include "TypePool.h"
#include "vm/core/DWARFLinker/Parallel/DWARFLinker.h"
#include "vm/core/DWARFLinker/StringPool.h"
#include "vm/core/Support/PerThreadBumpPtrAllocator.h"

namespace vm::core {

class DWARFDie;

namespace dwarf_linker {
namespace parallel {

using MessageHandlerTy = std::function<void(
    const Twine &Warning, StringRef Context, const DWARFDie *DIE)>;

/// linking options
struct DWARFLinkerOptions {
  /// DWARF version for the output.
  uint16_t TargetDWARFVersion = 0;

  /// Generate processing log to the standard output.
  bool Verbose = false;

  /// Print statistics.
  bool Statistics = false;

  /// Verify the input DWARF.
  bool VerifyInputDWARF = false;

  /// Do not unique types according to ODR
  bool NoODR = false;

  /// Update index tables.
  bool UpdateIndexTablesOnly = false;

  /// Whether we want a static variable to force us to keep its enclosing
  /// function.
  bool KeepFunctionForStatic = false;

  /// Allow to generate valid, but non deterministic output.
  bool AllowNonDeterministicOutput = false;

  /// Number of threads.
  unsigned Threads = 1;

  /// The accelerator table kinds
  SmallVector<DWARFLinkerBase::AccelTableKind, 1> AccelTables;

  /// Prepend path for the clang modules.
  std::string PrependPath;

  /// input verification handler(it might be called asynchronously).
  DWARFLinkerBase::InputVerificationHandlerTy InputVerificationHandler =
      nullptr;

  /// A list of all .swiftinterface files referenced by the debug
  /// info, mapping Module name to path on disk. The entries need to
  /// be uniqued and sorted and there are only few entries expected
  /// per compile unit, which is why this is a std::map.
  /// this is dsymutil specific fag.
  ///
  /// (it might be called asynchronously).
  DWARFLinkerBase::SwiftInterfacesMapTy *ParseableSwiftInterfaces = nullptr;

  /// A list of remappings to apply to file paths.
  ///
  /// (it might be called asynchronously).
  DWARFLinkerBase::ObjectPrefixMapTy *ObjectPrefixMap = nullptr;
};

class DWARFLinkerImpl;

/// This class keeps data and services common for the whole linking process.
class LinkingGlobalData {
  friend DWARFLinkerImpl;

public:
  /// Returns global per-thread allocator.
  toolchain::parallel::PerThreadBumpPtrAllocator &getAllocator() {
    return Allocator;
  }

  /// Returns global string pool.
  StringPool &getStringPool() { return Strings; }

  /// Returns linking options.
  const DWARFLinkerOptions &getOptions() const { return Options; }

  /// Set warning handler.
  void setWarningHandler(MessageHandlerTy Handler) { WarningHandler = Handler; }

  /// Set error handler.
  void setErrorHandler(MessageHandlerTy Handler) { ErrorHandler = Handler; }

  /// Report warning.
  void warn(const Twine &Warning, StringRef Context,
            const DWARFDie *DIE = nullptr) {
    if (WarningHandler)
      (WarningHandler)(Warning, Context, DIE);
  }

  /// Report warning.
  void warn(Error Warning, StringRef Context, const DWARFDie *DIE = nullptr) {
    handleAllErrors(std::move(Warning), [&](ErrorInfoBase &Info) {
      warn(Info.message(), Context, DIE);
    });
  }

  /// Report error.
  void error(const Twine &Err, StringRef Context,
             const DWARFDie *DIE = nullptr) {
    if (ErrorHandler)
      (ErrorHandler)(Err, Context, DIE);
  }

  /// Report error.
  void error(Error Err, StringRef Context, const DWARFDie *DIE = nullptr) {
    handleAllErrors(std::move(Err), [&](ErrorInfoBase &Info) {
      error(Info.message(), Context, DIE);
    });
  }

  /// Set target triple.
  void setTargetTriple(const Triple &TargetTriple) {
    this->TargetTriple = TargetTriple;
  }

  /// Optionally return target triple.
  std::optional<std::reference_wrapper<const Triple>> getTargetTriple() {
    if (TargetTriple)
      return std::cref(*TargetTriple);

    return std::nullopt;
  }

protected:
  toolchain::parallel::PerThreadBumpPtrAllocator Allocator;
  StringPool Strings;
  DWARFLinkerOptions Options;
  MessageHandlerTy WarningHandler;
  MessageHandlerTy ErrorHandler;

  /// Triple for output data. May be not set if generation of output
  /// data is not requested.
  std::optional<Triple> TargetTriple;
};

} // end of namespace parallel
} // end of namespace dwarf_linker
} // end of namespace vm::core

#endif // LLVM_LIB_DWARFLINKER_PARALLEL_DWARFLINKERGLOBALDATA_H
