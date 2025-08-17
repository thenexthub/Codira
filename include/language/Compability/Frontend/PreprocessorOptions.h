//===- PreprocessorOptions.h ------------------------------------*- C++ -*-===//
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
///
/// \file
/// This file contains the declaration of the PreprocessorOptions class, which
/// is the class for all preprocessor options.
///
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_FRONTEND_PREPROCESSOROPTIONS_H
#define LANGUAGE_COMPABILITY_FRONTEND_PREPROCESSOROPTIONS_H

#include "toolchain/ADT/StringRef.h"

namespace language::Compability::frontend {

/// Communicates whether to include/exclude predefined and command
/// line preprocessor macros
enum class PPMacrosFlag : uint8_t {
  /// Use the file extension to decide
  Unknown,

  Include,
  Exclude
};

/// This class is used for passing the various options used
/// in preprocessor initialization to the parser options.
struct PreprocessorOptions {
  PreprocessorOptions() {}

  std::vector<std::pair<std::string, /*isUndef*/ bool>> macros;

  // Search directories specified by the user with -I
  // TODO: When adding support for more options related to search paths,
  // consider collecting them in a separate aggregate. For now we keep it here
  // as there is no point creating a class for just one field.
  std::vector<std::string> searchDirectoriesFromDashI;
  // Search directories specified by the user with -fintrinsic-modules-path
  std::vector<std::string> searchDirectoriesFromIntrModPath;

  PPMacrosFlag macrosFlag = PPMacrosFlag::Unknown;

  // -P: Suppress #line directives in -E output
  bool noLineDirectives{false};

  // -fno-reformat: Emit cooked character stream as -E output
  bool noReformat{false};

  // -fpreprocess-include-lines: Treat INCLUDE as #include for -E output
  bool preprocessIncludeLines{false};

  // -dM: Show macro definitions with -dM -E
  bool showMacros{false};

  void addMacroDef(toolchain::StringRef name) {
    macros.emplace_back(std::string(name), false);
  }

  void addMacroUndef(toolchain::StringRef name) {
    macros.emplace_back(std::string(name), true);
  }
};

} // namespace language::Compability::frontend

#endif // FORTRAN_FRONTEND_PREPROCESSOROPTIONS_H
