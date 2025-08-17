//== Yaml.h ---------------------------------------------------- -*- C++ -*--=//
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
//
// This file defines convenience functions for handling YAML configuration files
// for checkers/packages.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKER_YAML_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKER_YAML_H

#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/YAMLTraits.h"
#include <optional>

namespace language::Core {
namespace ento {

/// Read the given file from the filesystem and parse it as a yaml file. The
/// template parameter must have a yaml MappingTraits.
/// Emit diagnostic error in case of any failure.
template <class T, class Checker>
std::optional<T> getConfiguration(CheckerManager &Mgr, Checker *Chk,
                                  StringRef Option, StringRef ConfigFile) {
  if (ConfigFile.trim().empty())
    return std::nullopt;

  toolchain::vfs::FileSystem *FS = toolchain::vfs::getRealFileSystem().get();
  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> Buffer =
      FS->getBufferForFile(ConfigFile.str());

  if (Buffer.getError()) {
    Mgr.reportInvalidCheckerOptionValue(Chk, Option,
                                        "a valid filename instead of '" +
                                            std::string(ConfigFile) + "'");
    return std::nullopt;
  }

  toolchain::yaml::Input Input(Buffer.get()->getBuffer());
  T Config;
  Input >> Config;

  if (std::error_code ec = Input.error()) {
    Mgr.reportInvalidCheckerOptionValue(Chk, Option,
                                        "a valid yaml file: " + ec.message());
    return std::nullopt;
  }

  return Config;
}

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_STATICANALYZER_CHECKER_YAML_H
