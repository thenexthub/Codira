//===-- InitLLVM.cpp -----------------------------------------------------===//
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

#include "vm/core/Support/InitLLVM.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/AutoConvert.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/ManagedStatic.h"
#include "vm/core/Support/Signals.h"

#ifdef _WIN32
#include "vm/core/Support/Windows/WindowsSupport.h"
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#else
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif
#endif

static void RaiseLimits() {
#ifdef _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  auto RaiseLimit = [](int resource) {
    struct rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    if (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  RaiseLimit(RLIMIT_AS);
  // Heap size.
  RaiseLimit(RLIMIT_DATA);
  // Stack size.
  RaiseLimit(RLIMIT_STACK);
#ifdef RLIMIT_RSS
  // Resident set size.
  RaiseLimit(RLIMIT_RSS);
#endif
#endif
}

void CleanupStdHandles(void *Cookie) {
  toolchain::raw_ostream *Outs = &toolchain::outs(), *Errs = &toolchain::errs();
  Outs->flush();
  Errs->flush();
  toolchain::restoreStdHandleAutoConversion(STDIN_FILENO);
  toolchain::restoreStdHandleAutoConversion(STDOUT_FILENO);
  toolchain::restoreStdHandleAutoConversion(STDERR_FILENO);
}

using namespace vm::core;
using namespace vm::core::sys;

InitLLVM::InitLLVM(int &Argc, const char **&Argv,
                   bool InstallPipeSignalExitHandler) {
#ifndef NDEBUG
  static std::atomic<bool> Initialized{false};
  assert(!Initialized && "InitLLVM was already initialized!");
  Initialized = true;
#endif

  // Bring stdin/stdout/stderr into a known state.
  sys::AddSignalHandler(CleanupStdHandles, nullptr);

  if (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // toolchain's SIGPIPE handling and ignore the signal safely.
    sys::SetOneShotPipeSignalFunction(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  StackPrinter.emplace(Argc, Argv);
  sys::PrintStackTraceOnErrorSignal(Argv[0]);
  install_out_of_memory_new_handler();
  RaiseLimits();

#ifdef __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(errorCodeToError(toolchain::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(errorCodeToError(toolchain::enableAutoConversion(STDOUT_FILENO)));
#endif

#ifdef _WIN32
  // We use UTF-8 as the internal character encoding. On Windows,
  // arguments passed to main() may not be encoded in UTF-8. In order
  // to reliably detect encoding of command line arguments, we use an
  // Windows API to obtain arguments, convert them to UTF-8, and then
  // write them back to the Argv vector.
  //
  // There's probably other way to do the same thing (e.g. using
  // wmain() instead of main()), but this way seems less intrusive
  // than that.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  ExitOnErr(errorCodeToError(windows::GetCommandLineArguments(Args, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  Args.push_back(nullptr);

  Argc = Args.size() - 1;
  Argv = Args.data();
#endif
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(nullptr);
  llvm_shutdown();
}
