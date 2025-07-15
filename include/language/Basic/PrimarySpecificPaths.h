//===--- PrimarySpecificPaths.h ---------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_PRIMARYSPECIFICPATHS_H
#define LANGUAGE_BASIC_PRIMARYSPECIFICPATHS_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/SupplementaryOutputPaths.h"
#include "toolchain/ADT/StringRef.h"

#include <string>

namespace language {

/// Holds all of the output paths, and debugging-info path that are
/// specific to which primary file is being compiled at the moment.

class PrimarySpecificPaths {
public:
  /// The name of the main output file,
  /// that is, the .o file for this input (or a file specified by -o).
  /// If there is no such file, contains an empty string. If the output
  /// is to be written to stdout, contains "-".
  std::string OutputFilename;

  /// The name to report the main output file as being in the index store.
  /// This is equivalent to OutputFilename, unless -index-store-output-path
  /// was specified.
  std::string IndexUnitOutputFilename;

  SupplementaryOutputPaths SupplementaryOutputs;

  /// The name of the "main" input file, used by the debug info.
  std::string MainInputFilenameForDebugInfo;

  PrimarySpecificPaths(StringRef OutputFilename = StringRef(),
                       StringRef IndexUnitOutputFilename = StringRef(),
                       StringRef MainInputFilenameForDebugInfo = StringRef(),
                       SupplementaryOutputPaths SupplementaryOutputs =
                           SupplementaryOutputPaths())
      : OutputFilename(OutputFilename),
        IndexUnitOutputFilename(IndexUnitOutputFilename),
        SupplementaryOutputs(SupplementaryOutputs),
        MainInputFilenameForDebugInfo(MainInputFilenameForDebugInfo) {}

  bool haveModuleOrModuleDocOutputPaths() const {
    return !SupplementaryOutputs.ModuleOutputPath.empty() ||
           !SupplementaryOutputs.ModuleDocOutputPath.empty();
  }
  bool haveModuleSummaryOutputPath() const {
    return !SupplementaryOutputs.ModuleSummaryOutputPath.empty();
  }
};
} // namespace language

#endif // LANGUAGE_BASIC_PRIMARYSPECIFICPATHS_H
