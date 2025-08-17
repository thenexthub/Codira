//===- CompilationDatabasePluginRegistry.h ----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_TOOLING_COMPILATIONDATABASEPLUGINREGISTRY_H
#define LANGUAGE_CORE_TOOLING_COMPILATIONDATABASEPLUGINREGISTRY_H

#include "language/Core/Support/Compiler.h"
#include "language/Core/Tooling/CompilationDatabase.h"
#include "toolchain/Support/Registry.h"

namespace language::Core {
namespace tooling {

/// Interface for compilation database plugins.
///
/// A compilation database plugin allows the user to register custom compilation
/// databases that are picked up as compilation database if the corresponding
/// library is linked in. To register a plugin, declare a static variable like:
///
/// \code
/// static CompilationDatabasePluginRegistry::Add<MyDatabasePlugin>
/// X("my-compilation-database", "Reads my own compilation database");
/// \endcode
class CompilationDatabasePlugin {
public:
  virtual ~CompilationDatabasePlugin();

  /// Loads a compilation database from a build directory.
  ///
  /// \see CompilationDatabase::loadFromDirectory().
  virtual std::unique_ptr<CompilationDatabase>
  loadFromDirectory(StringRef Directory, std::string &ErrorMessage) = 0;
};

using CompilationDatabasePluginRegistry =
    toolchain::Registry<CompilationDatabasePlugin>;

} // namespace tooling
} // namespace language::Core

namespace toolchain {
extern template class CLANG_TEMPLATE_ABI
    Registry<language::Core::tooling::CompilationDatabasePlugin>;
} // namespace toolchain

#endif // LANGUAGE_CORE_TOOLING_COMPILATIONDATABASEPLUGINREGISTRY_H
