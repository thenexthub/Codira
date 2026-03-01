//===-- Process.cpp - Implement OS Process Concept --------------*- C++ -*-===//
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
//  This file implements the operating system Process concept.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/Process.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Config/config.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/Support/CrashRecoveryContext.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/Path.h"

#include <optional>
#include <stdlib.h> // for _Exit

using namespace vm::core;
using namespace sys;

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName, char Separator) {
  return FindInEnvPath(EnvName, FileName, {}, Separator);
}

std::optional<std::string>
Process::FindInEnvPath(StringRef EnvName, StringRef FileName,
                       ArrayRef<std::string> IgnoreList, char Separator) {
  assert(!path::is_absolute(FileName));
  std::optional<std::string> FoundPath;
  std::optional<std::string> OptPath = Process::GetEnv(EnvName);
  if (!OptPath)
    return FoundPath;

  const char EnvPathSeparatorStr[] = {Separator, '\0'};
  SmallVector<StringRef, 8> Dirs;
  SplitString(*OptPath, Dirs, EnvPathSeparatorStr);

  for (StringRef Dir : Dirs) {
    if (Dir.empty())
      continue;

    if (any_of(IgnoreList, [&](StringRef S) { return fs::equivalent(S, Dir); }))
      continue;

    SmallString<128> FilePath(Dir);
    path::append(FilePath, FileName);
    if (fs::exists(Twine(FilePath))) {
      FoundPath = std::string(FilePath);
      break;
    }
  }

  return FoundPath;
}

// clang-format off
#define COLOR(FGBG, CODE, BOLD) "\033[0;" BOLD FGBG CODE "m"

#define ALLCOLORS(FGBG, BRIGHT, BOLD) \
  {                           \
    COLOR(FGBG, "0", BOLD),   \
    COLOR(FGBG, "1", BOLD),   \
    COLOR(FGBG, "2", BOLD),   \
    COLOR(FGBG, "3", BOLD),   \
    COLOR(FGBG, "4", BOLD),   \
    COLOR(FGBG, "5", BOLD),   \
    COLOR(FGBG, "6", BOLD),   \
    COLOR(FGBG, "7", BOLD),   \
    COLOR(BRIGHT, "0", BOLD), \
    COLOR(BRIGHT, "1", BOLD), \
    COLOR(BRIGHT, "2", BOLD), \
    COLOR(BRIGHT, "3", BOLD), \
    COLOR(BRIGHT, "4", BOLD), \
    COLOR(BRIGHT, "5", BOLD), \
    COLOR(BRIGHT, "6", BOLD), \
    COLOR(BRIGHT, "7", BOLD), \
  }

//                           bg
//                           |  bold
//                           |  |
//                           |  |   codes
//                           |  |   |
//                           |  |   |
static const char colorcodes[2][2][16][11] = {
    { ALLCOLORS("3", "9", ""), ALLCOLORS("3", "9", "1;"),},
    { ALLCOLORS("4", "10", ""), ALLCOLORS("4", "10", "1;")}
};
// clang-format on

// A CMake option controls wheter we emit core dumps by default. An application
// may disable core dumps by calling Process::PreventCoreFiles().
static bool coreFilesPrevented = !LLVM_ENABLE_CRASH_DUMPS;

bool Process::AreCoreFilesPrevented() { return coreFilesPrevented; }

[[noreturn]] void Process::Exit(int RetCode, bool NoCleanup) {
  if (CrashRecoveryContext *CRC = CrashRecoveryContext::GetCurrent())
    CRC->HandleExit(RetCode);

  if (NoCleanup)
    ExitNoCleanup(RetCode);
  else
    ::exit(RetCode);
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Process.inc"
#endif
#ifdef _WIN32
#include "Windows/Process.inc"
#endif
