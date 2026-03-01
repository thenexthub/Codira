//===-- Version.cpp -------------------------------------------------------===//
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

#include "lldb/Version/Version.h"
#include "VCSVersion.inc"
#include "lldb/Version/Version.inc"
#include "clang/Basic/Version.h"

static const char *GetLLDBVersion() {
#ifdef LLDB_FULL_VERSION_STRING
  return LLDB_FULL_VERSION_STRING;
#else
  return "lldb version " LLDB_VERSION_STRING;
#endif
}

static const char *GetLLDBRevision() {
#ifdef LLDB_REVISION
  return LLDB_REVISION;
#else
  return nullptr;
#endif
}

static const char *GetLLDBRepository() {
#ifdef LLDB_REPOSITORY
  return LLDB_REPOSITORY;
#else
  return nullptr;
#endif
}

const char *lldb_private::GetVersion() {
  static std::string g_version_str;

  if (g_version_str.empty()) {
    const char *lldb_version = GetLLDBVersion();
    const char *lldb_repo = GetLLDBRepository();
    const char *lldb_rev = GetLLDBRevision();
    g_version_str += lldb_version;
    if (lldb_repo || lldb_rev) {
      g_version_str += " (";
      if (lldb_repo)
        g_version_str += lldb_repo;
      if (lldb_repo && lldb_rev)
        g_version_str += " ";
      if (lldb_rev) {
        g_version_str += "revision ";
        g_version_str += lldb_rev;
      }
      g_version_str += ")";
    }

    std::string clang_rev(clang::getClangRevision());
    if (clang_rev.length() > 0) {
      g_version_str += "\n  clang revision ";
      g_version_str += clang_rev;
    }

    std::string llvm_rev(clang::getLLVMRevision());
    if (llvm_rev.length() > 0) {
      g_version_str += "\n  llvm revision ";
      g_version_str += llvm_rev;
    }
  }

  return g_version_str.c_str();
}
