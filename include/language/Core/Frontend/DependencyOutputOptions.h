//===--- DependencyOutputOptions.h ------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_FRONTEND_DEPENDENCYOUTPUTOPTIONS_H
#define LANGUAGE_CORE_FRONTEND_DEPENDENCYOUTPUTOPTIONS_H

#include "language/Core/Basic/HeaderInclude.h"
#include <string>
#include <vector>

namespace language::Core {

/// ShowIncludesDestination - Destination for /showIncludes output.
enum class ShowIncludesDestination { None, Stdout, Stderr };

/// DependencyOutputFormat - Format for the compiler dependency file.
enum class DependencyOutputFormat { Make, NMake };

/// ExtraDepKind - The kind of extra dependency file.
enum ExtraDepKind {
  EDK_SanitizeIgnorelist,
  EDK_ProfileList,
  EDK_ModuleFile,
  EDK_DepFileEntry,
};

/// DependencyOutputOptions - Options for controlling the compiler dependency
/// file generation.
class DependencyOutputOptions {
public:
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeSystemHeaders : 1; ///< Include system header dependencies.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowHeaderIncludes : 1;   ///< Show header inclusions (-H).
  LLVM_PREFERRED_TYPE(bool)
  unsigned UsePhonyTargets : 1;      ///< Include phony targets for each
                                     /// dependency, which can avoid some 'make'
                                     /// problems.
  LLVM_PREFERRED_TYPE(bool)
  unsigned AddMissingHeaderDeps : 1; ///< Add missing headers to dependency list
  LLVM_PREFERRED_TYPE(bool)
  unsigned IncludeModuleFiles : 1; ///< Include module file dependencies.
  LLVM_PREFERRED_TYPE(bool)
  unsigned ShowSkippedHeaderIncludes : 1; ///< With ShowHeaderIncludes, show
                                          /// also includes that were skipped
                                          /// due to the "include guard
                                          /// optimization" or #pragma once.

  /// The format of header information.
  HeaderIncludeFormatKind HeaderIncludeFormat = HIFMT_Textual;

  /// Determine whether header information should be filtered.
  HeaderIncludeFilteringKind HeaderIncludeFiltering = HIFIL_None;

  /// Destination of cl.exe style /showIncludes info.
  ShowIncludesDestination ShowIncludesDest = ShowIncludesDestination::None;

  /// The format for the dependency file.
  DependencyOutputFormat OutputFormat = DependencyOutputFormat::Make;

  /// The file to write dependency output to.
  std::string OutputFile;

  /// The file to write header include output to. This is orthogonal to
  /// ShowHeaderIncludes (-H) and will include headers mentioned in the
  /// predefines buffer. If the output file is "-", output will be sent to
  /// stderr.
  std::string HeaderIncludeOutputFile;

  /// A list of names to use as the targets in the dependency file; this list
  /// must contain at least one entry.
  std::vector<std::string> Targets;

  /// A list of extra dependencies (filename and kind) to be used for every
  /// target.
  std::vector<std::pair<std::string, ExtraDepKind>> ExtraDeps;

  /// The file to write GraphViz-formatted header dependencies to.
  std::string DOTOutputFile;

  /// The directory to copy module dependencies to when collecting them.
  std::string ModuleDependencyOutputDir;

public:
  DependencyOutputOptions()
      : IncludeSystemHeaders(0), ShowHeaderIncludes(0), UsePhonyTargets(0),
        AddMissingHeaderDeps(0), IncludeModuleFiles(0),
        ShowSkippedHeaderIncludes(0), HeaderIncludeFormat(HIFMT_Textual),
        HeaderIncludeFiltering(HIFIL_None) {}
};

}  // end namespace language::Core

#endif
