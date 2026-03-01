//===-- lldb-server.cpp -----------------------------------------*- C++ -*-===//
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

#include "SystemInitializerLLGS.h"
#include "lldb/Host/Config.h"
#include "lldb/Initialization/SystemLifetimeManager.h"
#include "lldb/Version/Version.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"

#include <cstdio>
#include <cstdlib>

static llvm::ManagedStatic<lldb_private::SystemLifetimeManager>
    g_debugger_lifetime;

static void display_usage(const char *progname) {
  fprintf(stderr, "Usage:\n"
                  "  %s v[ersion]\n"
                  "  %s g[dbserver] [options]\n"
                  "  %s p[latform] [options]\n"
                  "Invoke subcommand for additional help\n",
          progname, progname, progname);
  exit(0);
}

// Forward declarations of subcommand main methods.
int main_gdbserver(int argc, char *argv[]);
int main_platform(int argc, char *argv[]);

namespace llgs {
static void initialize() {
  if (auto e = g_debugger_lifetime->Initialize(
          std::make_unique<SystemInitializerLLGS>()))
    llvm::consumeError(std::move(e));
}

static void terminate_debugger() { g_debugger_lifetime->Terminate(); }
} // namespace llgs

// main
int main(int argc, char *argv[]) {
  llvm::InitLLVM IL(argc, argv, /*InstallPipeSignalExitHandler=*/false);
  llvm::setBugReportMsg("PLEASE submit a bug report to " LLDB_BUG_REPORT_URL
                        " and include the crash backtrace.\n");

  int option_error = 0;
  const char *progname = argv[0];
  if (argc < 2) {
    display_usage(progname);
    exit(option_error);
  }

  switch (argv[1][0]) {
  case 'g':
    llgs::initialize();
    main_gdbserver(argc, argv);
    llgs::terminate_debugger();
    break;
  case 'p':
    llgs::initialize();
    main_platform(argc, argv);
    llgs::terminate_debugger();
    break;
  case 'v':
    fprintf(stderr, "%s\n", lldb_private::GetVersion());
    break;
  default:
    display_usage(progname);
    exit(option_error);
  }
}
