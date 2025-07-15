//===--- Util.h - Common Driver Utilities -----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_DRIVER_UTIL_H
#define LANGUAGE_DRIVER_UTIL_H

#include "language/Basic/FileTypes.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/SmallVector.h"

namespace toolchain {
namespace opt {
  class Arg;
} // end namespace opt
} // end namespace toolchain

namespace language {

namespace driver {
  /// An input argument from the command line and its inferred type.
  using InputPair = std::pair<file_types::ID, const toolchain::opt::Arg *>;
  /// Type used for a list of input arguments.
  using InputFileList = SmallVector<InputPair, 16>;

  enum class LinkKind {
    None,
    Executable,
    DynamicLibrary,
    StaticLibrary
  };

  /// Used by a Job to request a "filelist": a file containing a list of all
  /// input or output files of a certain type.
  ///
  /// The Compilation is responsible for generating this file before running
  /// the Job this info is attached to.
  struct FilelistInfo {
    enum class WhichFiles : unsigned {
      InputJobs,
      SourceInputActions,
      InputJobsAndSourceInputActions,
      Output,
      IndexUnitOutputPaths,
      /// Batch mode frontend invocations may have so many supplementary
      /// outputs that they don't comfortably fit as command-line arguments.
      /// In that case, add a FilelistInfo to record the path to the file.
      /// The type is ignored.
      SupplementaryOutput,
    };

    StringRef path;
    file_types::ID type;
    WhichFiles whichFiles;
  };

} // end namespace driver
} // end namespace language

#endif
