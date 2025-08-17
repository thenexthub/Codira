//===- ArgumentsAdjusters.h - Command line arguments adjuster ---*- C++ -*-===//
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
// This file declares type ArgumentsAdjuster and functions to create several
// useful argument adjusters.
// ArgumentsAdjusters modify command line arguments obtained from a compilation
// database before they are used to run a frontend action.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_ARGUMENTSADJUSTERS_H
#define LANGUAGE_CORE_TOOLING_ARGUMENTSADJUSTERS_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"
#include <functional>
#include <string>
#include <vector>

namespace language::Core {
namespace tooling {

/// A sequence of command line arguments.
using CommandLineArguments = std::vector<std::string>;

/// A prototype of a command line adjuster.
///
/// Command line argument adjuster is responsible for command line arguments
/// modification before the arguments are used to run a frontend action.
using ArgumentsAdjuster = std::function<CommandLineArguments(
    const CommandLineArguments &, StringRef Filename)>;

/// Gets an argument adjuster that converts input command line arguments
/// to the "syntax check only" variant.
ArgumentsAdjuster getClangSyntaxOnlyAdjuster();

/// Gets an argument adjuster which removes output-related command line
/// arguments.
ArgumentsAdjuster getClangStripOutputAdjuster();

/// Gets an argument adjuster which removes dependency-file
/// related command line arguments.
ArgumentsAdjuster getClangStripDependencyFileAdjuster();

enum class ArgumentInsertPosition { BEGIN, END };

/// Gets an argument adjuster which inserts \p Extra arguments in the
/// specified position.
ArgumentsAdjuster getInsertArgumentAdjuster(const CommandLineArguments &Extra,
                                            ArgumentInsertPosition Pos);

/// Gets an argument adjuster which inserts an \p Extra argument in the
/// specified position.
ArgumentsAdjuster getInsertArgumentAdjuster(
    const char *Extra,
    ArgumentInsertPosition Pos = ArgumentInsertPosition::END);

/// Gets an argument adjuster which strips plugin related command line
/// arguments.
ArgumentsAdjuster getStripPluginsAdjuster();

/// Gets an argument adjuster which adjusts the arguments in sequence
/// with the \p First adjuster and then with the \p Second one.
ArgumentsAdjuster combineAdjusters(ArgumentsAdjuster First,
                                   ArgumentsAdjuster Second);

} // namespace tooling
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_ARGUMENTSADJUSTERS_H
