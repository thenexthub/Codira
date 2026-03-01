//===- LLDBTableGen.cpp - Top-Level TableGen implementation for LLDB ------===//
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
// This file contains the main function for LLDB's TableGen.
//
//===----------------------------------------------------------------------===//

#include "LLDBTableGenBackends.h" // Declares all backends.
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"

using namespace llvm;
using namespace lldb_private;

enum ActionType {
  PrintRecords,
  DumpJSON,
  GenOptionDefs,
  GenPropertyDefs,
  GenPropertyEnumDefs,
};

static cl::opt<ActionType> Action(
    cl::desc("Action to perform:"),
    cl::values(clEnumValN(PrintRecords, "print-records",
                          "Print all records to stdout (default)"),
               clEnumValN(DumpJSON, "dump-json",
                          "Dump all records as machine-readable JSON"),
               clEnumValN(GenOptionDefs, "gen-lldb-option-defs",
                          "Generate lldb option definitions"),
               clEnumValN(GenPropertyDefs, "gen-lldb-property-defs",
                          "Generate lldb property definitions"),
               clEnumValN(GenPropertyEnumDefs, "gen-lldb-property-enum-defs",
                          "Generate lldb property enum definitions")));

static bool LLDBTableGenMain(raw_ostream &OS, const RecordKeeper &Records) {
  switch (Action) {
  case PrintRecords:
    OS << Records; // No argument, dump all contents
    break;
  case DumpJSON:
    EmitJSON(Records, OS);
    break;
  case GenOptionDefs:
    EmitOptionDefs(Records, OS);
    break;
  case GenPropertyDefs:
    EmitPropertyDefs(Records, OS);
    break;
  case GenPropertyEnumDefs:
    EmitPropertyEnumDefs(Records, OS);
    break;
  }
  return false;
}

int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);
  llvm_shutdown_obj Y;

  return TableGenMain(argv[0], &LLDBTableGenMain);
}

#ifdef __has_feature
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>
// Disable LeakSanitizer for this binary as it has too many leaks that are not
// very interesting to fix. See compiler-rt/include/sanitizer/lsan_interface.h .
int __lsan_is_turned_off() { return 1; }
#endif // __has_feature(address_sanitizer)
#endif // defined(__has_feature)
