//===- PrintPasses.cpp ----------------------------------------------------===//
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

#include "vm/core/IR/PrintPasses.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Program.h"
#include <unordered_set>

using namespace vm::core;

// Print IR out before/after specified passes.
static cl::list<std::string>
    PrintBefore("print-before",
                toolchain::cl::desc("Print IR before specified passes"),
                cl::CommaSeparated, cl::Hidden);

static cl::list<std::string>
    PrintAfter("print-after", toolchain::cl::desc("Print IR after specified passes"),
               cl::CommaSeparated, cl::Hidden);

static cl::opt<bool> PrintBeforeAll("print-before-all",
                                    toolchain::cl::desc("Print IR before each pass"),
                                    cl::init(false), cl::Hidden);
static cl::opt<bool> PrintAfterAll("print-after-all",
                                   toolchain::cl::desc("Print IR after each pass"),
                                   cl::init(false), cl::Hidden);

// Print out the IR after passes, similar to -print-after-all except that it
// only prints the IR after passes that change the IR. Those passes that do not
// make changes to the IR are reported as not making any changes. In addition,
// the initial IR is also reported.  Other hidden options affect the output from
// this option. -filter-passes will limit the output to the named passes that
// actually change the IR and other passes are reported as filtered out. The
// specified passes will either be reported as making no changes (with no IR
// reported) or the changed IR will be reported. Also, the -filter-print-funcs
// and -print-module-scope options will do similar filtering based on function
// name, reporting changed IRs as functions(or modules if -print-module-scope is
// specified) for a particular function or indicating that the IR has been
// filtered out. The extra options can be combined, allowing only changed IRs
// for certain passes on certain functions to be reported in different formats,
// with the rest being reported as filtered out.  The -print-before-changed
// option will print the IR as it was before each pass that changed it. The
// optional value of quiet will only report when the IR changes, suppressing all
// other messages, including the initial IR. The values "diff" and "diff-quiet"
// will present the changes in a form similar to a patch, in either verbose or
// quiet mode, respectively. The lines that are removed and added are prefixed
// with '-' and '+', respectively. The -filter-print-funcs and -filter-passes
// can be used to filter the output.  This reporter relies on the linux diff
// utility to do comparisons and insert the prefixes. For systems that do not
// have the necessary facilities, the error message will be shown in place of
// the expected output.
cl::opt<ChangePrinter> toolchain::PrintChanged(
    "print-changed", cl::desc("Print changed IRs"), cl::Hidden,
    cl::ValueOptional, cl::init(ChangePrinter::None),
    cl::values(
        clEnumValN(ChangePrinter::Quiet, "quiet", "Run in quiet mode"),
        clEnumValN(ChangePrinter::DiffVerbose, "diff",
                   "Display patch-like changes"),
        clEnumValN(ChangePrinter::DiffQuiet, "diff-quiet",
                   "Display patch-like changes in quiet mode"),
        clEnumValN(ChangePrinter::ColourDiffVerbose, "cdiff",
                   "Display patch-like changes with color"),
        clEnumValN(ChangePrinter::ColourDiffQuiet, "cdiff-quiet",
                   "Display patch-like changes in quiet mode with color"),
        clEnumValN(ChangePrinter::DotCfgVerbose, "dot-cfg",
                   "Create a website with graphical changes"),
        clEnumValN(ChangePrinter::DotCfgQuiet, "dot-cfg-quiet",
                   "Create a website with graphical changes in quiet mode"),
        // Sentinel value for unspecified option.
        clEnumValN(ChangePrinter::Verbose, "", "")));

// An option for specifying the diff used by print-changed=[diff | diff-quiet]
static cl::opt<std::string>
    DiffBinary("print-changed-diff-path", cl::Hidden, cl::init("diff"),
               cl::desc("system diff used by change reporters"));

static cl::opt<bool>
    PrintModuleScope("print-module-scope",
                     cl::desc("When printing IR for print-[before|after]{-all} "
                              "always print a module IR"),
                     cl::init(false), cl::Hidden);

static cl::opt<bool> LoopPrintFuncScope(
    "print-loop-func-scope",
    cl::desc("When printing IR for print-[before|after]{-all} "
             "for a loop pass, always print function IR"),
    cl::init(false), cl::Hidden);

// See the description for -print-changed for an explanation of the use
// of this option.
static cl::list<std::string> FilterPasses(
    "filter-passes", cl::value_desc("pass names"),
    cl::desc("Only consider IR changes for passes whose names "
             "match the specified value. No-op without -print-changed"),
    cl::CommaSeparated, cl::Hidden);

static cl::list<std::string>
    PrintFuncsList("filter-print-funcs", cl::value_desc("function names"),
                   cl::desc("Only print IR for functions whose name "
                            "match this for all print-[before|after][-all] "
                            "options"),
                   cl::CommaSeparated, cl::Hidden);

/// This is a helper to determine whether to print IR before or
/// after a pass.

bool toolchain::shouldPrintBeforeSomePass() {
  return PrintBeforeAll || !PrintBefore.empty();
}

bool toolchain::shouldPrintAfterSomePass() {
  return PrintAfterAll || !PrintAfter.empty();
}

static bool shouldPrintBeforeOrAfterPass(StringRef PassID,
                                         ArrayRef<std::string> PassesToPrint) {
  return toolchain::is_contained(PassesToPrint, PassID);
}

bool toolchain::shouldPrintBeforeAll() { return PrintBeforeAll; }

bool toolchain::shouldPrintAfterAll() { return PrintAfterAll; }

bool toolchain::shouldPrintBeforePass(StringRef PassID) {
  return PrintBeforeAll || shouldPrintBeforeOrAfterPass(PassID, PrintBefore);
}

bool toolchain::shouldPrintAfterPass(StringRef PassID) {
  return PrintAfterAll || shouldPrintBeforeOrAfterPass(PassID, PrintAfter);
}

std::vector<std::string> toolchain::printBeforePasses() {
  return std::vector<std::string>(PrintBefore);
}

std::vector<std::string> toolchain::printAfterPasses() {
  return std::vector<std::string>(PrintAfter);
}

bool toolchain::forcePrintModuleIR() { return PrintModuleScope; }

bool toolchain::forcePrintFuncIR() { return LoopPrintFuncScope; }

bool toolchain::isPassInPrintList(StringRef PassName) {
  static std::unordered_set<std::string> Set(FilterPasses.begin(),
                                             FilterPasses.end());
  return Set.empty() || Set.count(std::string(PassName));
}

bool toolchain::isFilterPassesEmpty() { return FilterPasses.empty(); }

bool toolchain::isFunctionInPrintList(StringRef FunctionName) {
  static std::unordered_set<std::string> PrintFuncNames(PrintFuncsList.begin(),
                                                        PrintFuncsList.end());
  return PrintFuncNames.empty() ||
         PrintFuncNames.count(std::string(FunctionName));
}

std::error_code cleanUpTempFilesImpl(ArrayRef<std::string> FileName,
                                     unsigned N) {
  std::error_code RC;
  for (unsigned I = 0; I < N; ++I) {
    std::error_code EC = sys::fs::remove(FileName[I]);
    if (EC)
      RC = EC;
  }
  return RC;
}

std::error_code toolchain::prepareTempFiles(SmallVector<int> &FD,
                                       ArrayRef<StringRef> SR,
                                       SmallVector<std::string> &FileName) {
  assert(FD.size() >= SR.size() && FileName.size() == FD.size() &&
         "Unexpected array sizes");
  std::error_code EC;
  unsigned I = 0;
  for (; I < FD.size(); ++I) {
    if (FD[I] == -1) {
      SmallVector<char, 200> SV;
      EC = sys::fs::createTemporaryFile("tmpfile", "txt", FD[I], SV);
      if (EC)
        break;
      FileName[I] = Twine(SV).str();
    }
    if (I < SR.size()) {
      EC = sys::fs::openFileForWrite(FileName[I], FD[I]);
      if (EC)
        break;
      raw_fd_ostream OutStream(FD[I], /*shouldClose=*/true);
      if (FD[I] == -1) {
        EC = make_error_code(errc::io_error);
        break;
      }
      OutStream << SR[I];
    }
  }
  if (EC && I > 0)
    // clean up created temporary files
    cleanUpTempFilesImpl(FileName, I);
  return EC;
}

std::error_code toolchain::cleanUpTempFiles(ArrayRef<std::string> FileName) {
  return cleanUpTempFilesImpl(FileName, FileName.size());
}

std::string toolchain::doSystemDiff(StringRef Before, StringRef After,
                               StringRef OldLineFormat, StringRef NewLineFormat,
                               StringRef UnchangedLineFormat) {
  // Store the 2 bodies into temporary files and call diff on them
  // to get the body of the node.
  static SmallVector<int> FD{-1, -1, -1};
  SmallVector<StringRef> SR{Before, After};
  static SmallVector<std::string> FileName{"", "", ""};
  if (prepareTempFiles(FD, SR, FileName))
    return "Unable to create temporary file.";

  static ErrorOr<std::string> DiffExe = sys::findProgramByName(DiffBinary);
  if (!DiffExe)
    return "Unable to find diff executable.";

  SmallString<128> OLF, NLF, ULF;
  ("--old-line-format=" + OldLineFormat).toVector(OLF);
  ("--new-line-format=" + NewLineFormat).toVector(NLF);
  ("--unchanged-line-format=" + UnchangedLineFormat).toVector(ULF);

  StringRef Args[] = {DiffBinary, "-w", "-d",        OLF,
                      NLF,        ULF,  FileName[0], FileName[1]};
  std::optional<StringRef> Redirects[] = {std::nullopt, StringRef(FileName[2]),
                                          std::nullopt};
  int Result = sys::ExecuteAndWait(*DiffExe, Args, std::nullopt, Redirects);
  if (Result < 0)
    return "Error executing system diff.";
  std::string Diff;
  auto B = MemoryBuffer::getFile(FileName[2]);
  if (B && *B)
    Diff = (*B)->getBuffer().str();
  else
    return "Unable to read result.";

  if (cleanUpTempFiles(FileName))
    return "Unable to remove temporary file.";

  return Diff;
}
