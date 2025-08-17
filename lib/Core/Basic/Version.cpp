//===- Version.cpp - Clang Version Number -----------------------*- C++ -*-===//
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
// This file defines several version-related utility functions for Clang.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/Version.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Config/config.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdlib>
#include <cstring>

#include "VCSVersion.inc"

namespace language::Core {

std::string getClangRepositoryPath() {
#if defined(CLANG_REPOSITORY_STRING)
  return CLANG_REPOSITORY_STRING;
#else
#ifdef CLANG_REPOSITORY
  return CLANG_REPOSITORY;
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

std::string getClangRevision() {
#ifdef CLANG_REVISION
  return CLANG_REVISION;
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

std::string getClangVendor() {
#ifdef CLANG_VENDOR
  return CLANG_VENDOR;
#else
  return "";
#endif
}

std::string getClangFullRepositoryVersion() {
  std::string buf;
  toolchain::raw_string_ostream OS(buf);
  std::string Path = getClangRepositoryPath();
  std::string Revision = getClangRevision();
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

std::string getClangFullVersion() {
  return getClangToolFullVersion("clang");
}

std::string getClangToolFullVersion(StringRef ToolName) {
  std::string buf;
  toolchain::raw_string_ostream OS(buf);
  OS << getClangVendor() << ToolName << " version " CLANG_VERSION_STRING;

  std::string repo = getClangFullRepositoryVersion();
  if (!repo.empty()) {
    OS << " " << repo;
  }

  return buf;
}

std::string getClangFullCPPVersion() {
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  toolchain::raw_string_ostream OS(buf);
  OS << getClangVendor() << "Clang " CLANG_VERSION_STRING;

  std::string repo = getClangFullRepositoryVersion();
  if (!repo.empty()) {
    OS << " " << repo;
  }

  return buf;
}

} // end namespace language::Core
