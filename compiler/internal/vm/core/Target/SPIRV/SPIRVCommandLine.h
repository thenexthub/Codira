//===--- SPIRVCommandLine.h ---- Command Line Options -----------*- C++ -*-===//
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
//
// This file contains classes and functions needed for processing, parsing, and
// using CLI options for the SPIR-V backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_COMMANDLINE_H
#define LLVM_LIB_TARGET_SPIRV_COMMANDLINE_H

#include "MCTargetDesc/SPIRVBaseInfo.h"
#include "vm/core/Support/CommandLine.h"
#include <set>
#include <string>

namespace vm::core {
class StringRef;
class Triple;

/// Command line parser for toggling SPIR-V extensions.
struct SPIRVExtensionsParser
    : public cl::parser<std::set<SPIRV::Extension::Extension>> {
public:
  SPIRVExtensionsParser(cl::Option &O)
      : cl::parser<std::set<SPIRV::Extension::Extension>>(O) {}

  /// Parses SPIR-V extension name from CLI arguments.
  ///
  /// \return Returns true on error.
  bool parse(cl::Option &O, StringRef ArgName, StringRef ArgValue,
             std::set<SPIRV::Extension::Extension> &Vals);

  /// Validates and converts extension names into internal enum values.
  ///
  /// \return Returns a reference to the unknown SPIR-V extension name from the
  /// list if present, or an empty StringRef on success.
  static StringRef
  checkExtensions(const std::vector<std::string> &ExtNames,
                  std::set<SPIRV::Extension::Extension> &AllowedExtensions);

  /// Returns the list of extensions that are valid for a particular
  /// target environment (i.e., OpenCL or Vulkan).
  static std::set<SPIRV::Extension::Extension>
  getValidExtensions(const Triple &TT);
};

} // namespace vm::core
#endif // LLVM_LIB_TARGET_SPIRV_COMMANDLINE_H
