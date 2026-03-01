//===-- Config.h -----------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_CONFIG_H
#define LLDB_HOST_CONFIG_H

#cmakedefine01 LLDB_EDITLINE_USE_WCHAR

#cmakedefine01 LLDB_HAVE_EL_RFUNC_T

#cmakedefine01 HAVE_SYS_EVENT_H

#cmakedefine01 HAVE_PPOLL

#cmakedefine01 HAVE_PTSNAME_R

#cmakedefine01 HAVE_PROCESS_VM_READV

#cmakedefine01 HAVE_NR_PROCESS_VM_READV

#cmakedefine01 HAVE_LIBCOMPRESSION

#cmakedefine01 LLDB_ENABLE_POSIX

#cmakedefine01 LLDB_ENABLE_TERMIOS

#cmakedefine01 LLDB_ENABLE_LZMA

#cmakedefine01 LLVM_ENABLE_CURL

#cmakedefine01 LLDB_ENABLE_CURSES

#cmakedefine01 CURSES_HAVE_NCURSES_CURSES_H

#cmakedefine01 LLDB_ENABLE_LIBEDIT

#cmakedefine01 LLDB_ENABLE_LIBXML2

#cmakedefine01 LLDB_ENABLE_LUA

#cmakedefine01 LLDB_ENABLE_PYTHON

#cmakedefine01 LLDB_ENABLE_PYTHON_LIMITED_API

#cmakedefine01 LLDB_ENABLE_FBSDVMCORE

#cmakedefine01 LLDB_EMBED_PYTHON_HOME

#cmakedefine LLDB_PYTHON_HOME R"(${LLDB_PYTHON_HOME})"

#define LLDB_INSTALL_LIBDIR_BASENAME "${LLDB_INSTALL_LIBDIR_BASENAME}"

#cmakedefine LLDB_GLOBAL_INIT_DIRECTORY R"(${LLDB_GLOBAL_INIT_DIRECTORY})"

#define LLDB_BUG_REPORT_URL "${LLDB_BUG_REPORT_URL}"

#endif // #ifndef LLDB_HOST_CONFIG_H
