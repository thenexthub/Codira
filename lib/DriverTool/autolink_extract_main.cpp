//===--- autolink_extract_main.cpp - autolink extraction utility ----------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Extracts autolink flags from object files so they can be passed to the
// linker directly. Mostly useful for platforms where the linker doesn't
// natively support autolinking (ie. ELF-based platforms).
//
//===----------------------------------------------------------------------===//

#include <string>
#include <vector>

#include "language/AST/DiagnosticsFrontend.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Option/Options.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Option/Option.h"
#include "toolchain/Object/Archive.h"
#include "toolchain/Object/ObjectFile.h"
#include "toolchain/Object/ELFObjectFile.h"
#include "toolchain/Object/IRObjectFile.h"
#include "toolchain/Object/Wasm.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/BinaryFormat/Wasm.h"

using namespace language;
using namespace toolchain::opt;

class AutolinkExtractInvocation {
private:
  std::string MainExecutablePath;
  std::string OutputFilename = "-";
  std::vector<std::string> InputFilenames;

public:
  void setMainExecutablePath(const std::string &Path) {
    MainExecutablePath = Path;
  }

  const std::string &getOutputFilename() {
    return OutputFilename;
  }

  const std::vector<std::string> &getInputFilenames() {
    return InputFilenames;
  }

  int parseArgs(ArrayRef<const char *> Args,
                DiagnosticEngine &Diags) {
    using namespace options;

    // Parse frontend command line options using Codira's option table.
    std::unique_ptr<toolchain::opt::OptTable> Table = createCodiraOptTable();
    unsigned MissingIndex;
    unsigned MissingCount;
    toolchain::opt::InputArgList ParsedArgs =
      Table->ParseArgs(Args, MissingIndex, MissingCount, AutolinkExtractOption);
    if (MissingCount) {
      Diags.diagnose(SourceLoc(), diag::error_missing_arg_value,
                     ParsedArgs.getArgString(MissingIndex), MissingCount);
      return 1;
    }

    if (ParsedArgs.hasArg(OPT_UNKNOWN)) {
      for (const Arg *A : ParsedArgs.filtered(OPT_UNKNOWN)) {
        Diags.diagnose(SourceLoc(), diag::error_unknown_arg,
                       A->getAsString(ParsedArgs));
      }
      return true;
    }

    if (ParsedArgs.getLastArg(OPT_help)) {
      std::string ExecutableName =
          toolchain::sys::path::stem(MainExecutablePath).str();
      Table->printHelp(toolchain::outs(), ExecutableName.c_str(),
                       "Codira Autolink Extract", options::AutolinkExtractOption,
                       0, /*ShowAllAliases*/false);
      return 1;
    }

    for (const Arg *A : ParsedArgs.filtered(OPT_INPUT)) {
      InputFilenames.push_back(A->getValue());
    }

    if (InputFilenames.empty()) {
      Diags.diagnose(SourceLoc(), diag::error_mode_requires_an_input_file);
      return 1;
    }

    if (const Arg *A = ParsedArgs.getLastArg(OPT_o)) {
      OutputFilename = A->getValue();
    }

    return 0;
  }
};

/// Look inside the object file 'ObjectFile' and append any linker flags found in
/// its ".code1_autolink_entries" section to 'LinkerFlags'.
/// Return 'true' if there was an error, and 'false' otherwise.
static bool
extractLinkerFlagsFromObjectFile(const toolchain::object::ObjectFile *ObjectFile,
                                 std::vector<std::string> &LinkerFlags,
                                 std::unordered_map<std::string, bool> &CodiraRuntimeLibraries,
                                 CompilerInstance &Instance) {
  // Search for the section we hold autolink entries in
  for (auto &Section : ObjectFile->sections()) {
    toolchain::Expected<toolchain::StringRef> SectionNameOrErr = Section.getName();
    if (!SectionNameOrErr) {
      toolchain::consumeError(SectionNameOrErr.takeError());
      continue;
    }
    toolchain::StringRef SectionName = *SectionNameOrErr;
    if (SectionName == ".code1_autolink_entries") {
      toolchain::Expected<toolchain::StringRef> SectionData = Section.getContents();
      if (!SectionData) {
        std::string message;
        {
          toolchain::raw_string_ostream os(message);
          logAllUnhandledErrors(SectionData.takeError(), os, "");
        }
        Instance.getDiags().diagnose(SourceLoc(), diag::error_open_input_file,
                                  ObjectFile->getFileName() , message);
        return true;
      }

      // entries are null-terminated, so extract them and push them into
      // the set.
      toolchain::SmallVector<toolchain::StringRef, 4> SplitFlags;
      SectionData->split(SplitFlags, toolchain::StringRef("\0", 1), -1,
                         /*KeepEmpty=*/false);
      for (const auto &Flag : SplitFlags) {
        auto RuntimeLibEntry = CodiraRuntimeLibraries.find(Flag.str());
        if (RuntimeLibEntry == CodiraRuntimeLibraries.end())
          LinkerFlags.emplace_back(Flag.str());
        else
          RuntimeLibEntry->second = true;
      }
    }
  }
  return false;
}

/// Look inside the binary 'Bin' and append any linker flags found in its
/// ".code1_autolink_entries" section to 'LinkerFlags'. If 'Bin' is an archive,
/// recursively look inside all children within the archive. Return 'true' if
/// there was an error, and 'false' otherwise.
static bool extractLinkerFlags(const toolchain::object::Binary *Bin,
                               CompilerInstance &Instance,
                               StringRef BinaryFileName,
                               std::vector<std::string> &LinkerFlags,
                               std::unordered_map<std::string, bool> &CodiraRuntimeLibraries,
                               toolchain::LLVMContext *LLVMCtx) {
  if (auto *ObjectFile = toolchain::dyn_cast<toolchain::object::ELFObjectFileBase>(Bin)) {
    return extractLinkerFlagsFromObjectFile(ObjectFile, LinkerFlags, CodiraRuntimeLibraries, Instance);
  } else if (auto *ObjectFile =
                 toolchain::dyn_cast<toolchain::object::WasmObjectFile>(Bin)) {
    return extractLinkerFlagsFromObjectFile(ObjectFile, LinkerFlags, CodiraRuntimeLibraries, Instance);
  } else if (auto *Archive = toolchain::dyn_cast<toolchain::object::Archive>(Bin)) {
    toolchain::Error Error = toolchain::Error::success();
    for (const auto &Child : Archive->children(Error)) {
      auto ChildBinary = Child.getAsBinary(LLVMCtx);
      // FIXME: BinaryFileName below should instead be ld-style names for
      // object files in archives, e.g. "foo.a(bar.o)".
      if (!ChildBinary) {
        Instance.getDiags().diagnose(SourceLoc(), diag::error_open_input_file,
                                     BinaryFileName,
                                     toolchain::toString(ChildBinary.takeError()));
        return true;
      }
      if (extractLinkerFlags(ChildBinary->get(), Instance, BinaryFileName,
                             LinkerFlags, CodiraRuntimeLibraries, LLVMCtx)) {
        return true;
      }
    }
    return bool(Error);
  } else if (toolchain::isa<toolchain::object::IRObjectFile>(Bin)) {
    // Ignore the LLVM IR files (LTO)
    return false;
  }  else {
    Instance.getDiags().diagnose(SourceLoc(), diag::error_open_input_file,
                                 BinaryFileName,
                                 "Don't know how to extract from object file"
                                 "format");
    return true;
  }
}

int autolink_extract_main(ArrayRef<const char *> Args, const char *Argv0,
                          void *MainAddr) {
  CompilerInstance Instance;
  PrintingDiagnosticConsumer PDC;
  Instance.addDiagnosticConsumer(&PDC);

  AutolinkExtractInvocation Invocation;
  std::string MainExecutablePath = toolchain::sys::fs::getMainExecutable(Argv0,
                                                                    MainAddr);
  Invocation.setMainExecutablePath(MainExecutablePath);

  // Parse arguments.
  if (Invocation.parseArgs(Args, Instance.getDiags()) != 0) {
    return 1;
  }

  std::vector<std::string> LinkerFlags;

  // Keep track of whether we've already added the common
  // Codira libraries that usually have autolink directives
  // in most object files

  std::vector<std::string> CodiraRuntimeLibsOrdered = {
      // Common Codira runtime libs
      "-llanguageCodiraOnoneSupport",
      "-llanguageCore",
      "-llanguage_Concurrency",
      "-llanguage_StringProcessing",
      "-llanguageRegexBuilder",
      "-llanguage_RegexParser",
      "-llanguage_Builtin_float",
      "-llanguage_math",
      "-llanguageRuntime",
      "-llanguageSynchronization",
      "-llanguageGlibc",
      "-llanguageAndroid",
      "-lBlocksRuntime",
      // Dispatch-specific Codira runtime libs
      "-ldispatch",
      "-lDispatchStubs",
      "-llanguageDispatch",
      // CoreFoundation and Foundation Codira runtime libs
      "-l_FoundationICU",
      "-lCoreFoundation",
      "-lFoundation",
      "-lFoundationEssentials",
      "-lFoundationInternationalization",
      "-lFoundationNetworking",
      "-lFoundationXML",
      // Foundation support libs
      "-lcurl",
      "-lxml2",
      "-luuid",
      "-lTesting",
      // XCTest runtime libs (must be first due to http://github.com/apple/language-corelibs-xctest/issues/432)
      "-lXCTest",
      // Common-use ordering-agnostic Linux system libs
      "-lm",
      "-lpthread",
      "-lutil",
      "-ldl",
      "-lz",
  };
  std::unordered_map<std::string, bool> CodiraRuntimeLibraries;
  for (const auto &RuntimeLib : CodiraRuntimeLibsOrdered) {
    CodiraRuntimeLibraries[RuntimeLib] = false;
  }

  // Extract the linker flags from the objects.
  toolchain::LLVMContext LLVMCtx;
  for (const auto &BinaryFileName : Invocation.getInputFilenames()) {
    auto BinaryOwner = toolchain::object::createBinary(BinaryFileName, &LLVMCtx);
    if (!BinaryOwner) {
      std::string message;
      {
        toolchain::raw_string_ostream os(message);
        logAllUnhandledErrors(BinaryOwner.takeError(), os, "");
      }

      Instance.getDiags().diagnose(SourceLoc(), diag::error_open_input_file,
                                   BinaryFileName, message);
      return 1;
    }

    if (extractLinkerFlags(BinaryOwner->getBinary(), Instance, BinaryFileName,
                           LinkerFlags, CodiraRuntimeLibraries, &LLVMCtx)) {
      return 1;
    }
  }

  std::string OutputFilename = Invocation.getOutputFilename();
  std::error_code EC;
  toolchain::raw_fd_ostream OutOS(OutputFilename, EC, toolchain::sys::fs::OF_None);
  if (OutOS.has_error() || EC) {
    Instance.getDiags().diagnose(SourceLoc(), diag::error_opening_output,
                                 OutputFilename, EC.message());
    OutOS.clear_error();
    return 1;
  }

  for (auto &Flag : LinkerFlags) {
    OutOS << Flag << '\n';
  }

  for (const auto &RuntimeLib : CodiraRuntimeLibsOrdered) {
    auto entry = CodiraRuntimeLibraries.find(RuntimeLib);
    if (entry != CodiraRuntimeLibraries.end() && entry->second) {
      OutOS << entry->first << '\n';
    }
  }


  return 0;
}
