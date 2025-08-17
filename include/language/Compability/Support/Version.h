//===-- language/Compability/Support/Version.h -------------------------*- C++ -*-===//
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
/// Defines version macros and version-related utility functions
/// for Flang.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_SUPPORT_VERSION_H_
#define LANGUAGE_COMPABILITY_SUPPORT_VERSION_H_

#include "language/Compability/Version.inc"
#include "toolchain/ADT/StringRef.h"

namespace language::Compability::common {
/// Retrieves the repository path (e.g., Git path) that
/// identifies the particular Flang branch, tag, or trunk from which this
/// Flang was built.
std::string getFlangRepositoryPath();

/// Retrieves the repository path from which LLVM was built.
///
/// This supports LLVM residing in a separate repository from flang.
std::string getLLVMRepositoryPath();

/// Retrieves the repository revision number (or identifier) from which
/// this Flang was built.
std::string getFlangRevision();

/// Retrieves the repository revision number (or identifier) from which
/// LLVM was built.
///
/// If Flang and LLVM are in the same repository, this returns the same
/// string as getFlangRevision.
std::string getLLVMRevision();

/// Retrieves the full repository version that is an amalgamation of
/// the information in getFlangRepositoryPath() and getFlangRevision().
std::string getFlangFullRepositoryVersion();

/// Retrieves a string representing the complete flang version,
/// which includes the flang version number, the repository version,
/// and the vendor tag.
std::string getFlangFullVersion();

/// Like getFlangFullVersion(), but with a custom tool name.
std::string getFlangToolFullVersion(toolchain::StringRef ToolName);
} // namespace language::Compability::common

#endif // FORTRAN_SUPPORT_VERSION_H_
