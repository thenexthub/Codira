//===-- Platform.cpp --------------------------------------------*- C++ -*-===//
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

// this file is only relevant for Visual C++
#if defined(_WIN32)

#include <cassert>
#include <cstdlib>
#include <process.h>

#include "Platform.h"
#include "llvm/Support/ErrorHandling.h"

int ioctl(int d, int request, ...) {
  switch (request) {
  // request the console windows size
  case (TIOCGWINSZ): {
    va_list vl;
    va_start(vl, request);
    // locate the window size structure on stack
    winsize *ws = va_arg(vl, winsize *);
    // get screen buffer information
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info) ==
        TRUE)
      // fill in the columns
      ws->ws_col = info.dwMaximumWindowSize.X;
    va_end(vl);
    return 0;
  } break;
  default:
    llvm_unreachable("Not implemented!");
  }
}

int kill(pid_t pid, int sig) {
  // is the app trying to kill itself
  if (pid == getpid())
    exit(sig);
  //
  llvm_unreachable("Not implemented!");
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p) {
  llvm_unreachable("Not implemented!");
}

int tcgetattr(int fildes, struct termios *termios_p) {
  //  assert( !"Not implemented!" );
  // error return value (0=success)
  return -1;
}

#endif
