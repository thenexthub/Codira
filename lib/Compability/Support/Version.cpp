//===-- lib/Support/Version.cpp ---------------------------------*- C++ -*-===//
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
// This file defines several version-related utility functions for Flang.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Support/Version.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdlib>
#include <cstring>

#include "VCSVersion.inc"

namespace language::Compability::common {

std::string getFlangRepositoryPath() {
#if defined(FLANG_REPOSITORY_STRING)
  return FLANG_REPOSITORY_STRING;
#else
#ifdef FLANG_REPOSITORY
  return FLANG_REPOSITORY;
#else
  return "";
#endif
#endif
}

std::string getLLVMRepositoryPath() {
#ifdef LLVM_REPOSITORY
  return LLVM_REPOSITORY;
#else
  return "";
#endif
}

std::string getFlangRevision() {
#ifdef FLANG_REVISION
  return FLANG_REVISION;
#else
  return "";
#endif
}

std::string getLLVMRevision() {
#ifdef LLVM_REVISION
  return LLVM_REVISION;
#else
  return "";
#endif
}

std::string getFlangFullRepositoryVersion() {
  std::string buf;
  toolchain::raw_string_ostream OS(buf);
  std::string Path = getFlangRepositoryPath();
  std::string Revision = getFlangRevision();
  if (!Path.empty() || !Revision.empty()) {
    OS << '(';
    if (!Path.empty())
      OS << Path;
    if (!Revision.empty()) {
      if (!Path.empty())
        OS << ' ';
      OS << Revision;
    }
    OS << ')';
  }
  // Support LLVM in a separate repository.
  std::string LLVMRev = getLLVMRevision();
  if (!LLVMRev.empty() && LLVMRev != Revision) {
    OS << " (";
    std::string LLVMRepo = getLLVMRepositoryPath();
    if (!LLVMRepo.empty())
      OS << LLVMRepo << ' ';
    OS << LLVMRev << ')';
  }
  return buf;
}

std::string getFlangFullVersion() { return getFlangToolFullVersion("flang"); }

std::string getFlangToolFullVersion(toolchain::StringRef ToolName) {
  std::string buf;
  toolchain::raw_string_ostream OS(buf);
#ifdef FLANG_VENDOR
  OS << FLANG_VENDOR;
#endif
  OS << ToolName << " version " FLANG_VERSION_STRING;

  std::string repo = getFlangFullRepositoryVersion();
  if (!repo.empty()) {
    OS << " " << repo;
  }

  return buf;
}

} // end namespace language::Compability::common
