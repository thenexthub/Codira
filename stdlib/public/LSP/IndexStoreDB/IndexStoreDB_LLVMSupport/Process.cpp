//===-- Process.cpp - Implement OS Process Concept --------------*- C++ -*-===//
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
//  This file implements the operating system Process concept.
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_LLVMSupport/toolchain_Support_Process.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_STLExtras.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringExtras.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Config_toolchain-config.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Config_config.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_FileSystem.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Path.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Program.h>

using namespace toolchain;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

Optional<std::string> Process::FindInEnvPath(StringRef EnvName,
                                             StringRef FileName) {
  return FindInEnvPath(EnvName, FileName, {});
}

Optional<std::string> Process::FindInEnvPath(StringRef EnvName,
                                             StringRef FileName,
                                             ArrayRef<std::string> IgnoreList) {
  assert(!path::is_absolute(FileName));
  Optional<std::string> FoundPath;
  Optional<std::string> OptPath = Process::GetEnv(EnvName);
  if (!OptPath.hasValue())
    return FoundPath;

  const char EnvPathSeparatorStr[] = {EnvPathSeparator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(OptPath.getValue(), Dirs, EnvPathSeparatorStr);

  for (StringRef Dir : Dirs) {
    if (Dir.empty())
      continue;

    if (any_of(IgnoreList, [&](StringRef S) { return fs::equivalent(S, Dir); }))
      continue;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    if (fs::exists(Twine(FilePath))) {
      FoundPath = FilePath.str();
      break;
    }
  }

  return FoundPath;
}


#define COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#define ALLCOLORS(FGBG,BOLD) {\
    COLOR(FGBG, "0", BOLD),\
    COLOR(FGBG, "1", BOLD),\
    COLOR(FGBG, "2", BOLD),\
    COLOR(FGBG, "3", BOLD),\
    COLOR(FGBG, "4", BOLD),\
    COLOR(FGBG, "5", BOLD),\
    COLOR(FGBG, "6", BOLD),\
    COLOR(FGBG, "7", BOLD)\
  }

static const char colorcodes[2][2][8][10] = {
 { ALLCOLORS("3",""), ALLCOLORS("3","1;") },
 { ALLCOLORS("4",""), ALLCOLORS("4","1;") }
};

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
static bool coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

bool Process::AreCoreFilesPrevented() { return coreFilesPrevented; }

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Process.inc"
#endif
#ifdef _WIN32
#include "Windows/Process.inc"
#endif
