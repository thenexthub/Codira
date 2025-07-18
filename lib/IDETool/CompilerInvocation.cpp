//===--- CompilerInvocation.cpp - Compiler invocation utilities -----------===//
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

#include "language/IDETool/CompilerInvocation.h"

#include "language/Basic/Assertions.h"
#include "language/Driver/FrontendUtil.h"
#include "language/Frontend/Frontend.h"
#include "clang/AST/DeclObjC.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/PCHContainerOperations.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Serialization/ASTReader.h"

using namespace language;

static void disableExpensiveSILOptions(SILOptions &Opts) {
  // Disable the sanitizers.
  Opts.Sanitizers = {};

  // Disable PGO and code coverage.
  Opts.GenerateProfile = false;
  Opts.EmitProfileCoverageMapping = false;
  Opts.UseProfile = "";
}

// Adjust the cc1 triple string we got from clang, to make sure it will be
// accepted when it goes through the language clang importer.
static std::string adjustClangTriple(StringRef TripleStr) {
  std::string Result;
  toolchain::raw_string_ostream OS(Result);

  toolchain::Triple Triple(TripleStr);
  switch (Triple.getSubArch()) {
  case toolchain::Triple::SubArchType::ARMSubArch_v7:
    OS << "armv7"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v7s:
    OS << "armv7s"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v7k:
    OS << "armv7k"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v6:
    OS << "armv6"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v6m:
    OS << "armv6m"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v6k:
    OS << "armv6k"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v6t2:
    OS << "armv6t2"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v5:
    OS << "armv5"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v5te:
    OS << "armv5te"; break;
  case toolchain::Triple::SubArchType::ARMSubArch_v4t:
    OS << "armv4t"; break;
  default:
    // Adjust i386-macosx to x86_64 because there is no Codira stdlib for i386.
    if ((Triple.getOS() == toolchain::Triple::MacOSX ||
         Triple.getOS() == toolchain::Triple::Darwin) &&
        Triple.getArch() == toolchain::Triple::x86) {
      OS << "x86_64";
    } else {
      OS << Triple.getArchName();
    }
    break;
  }
  OS << '-' << Triple.getVendorName() << '-'
     << Triple.getOSAndEnvironmentName();
  OS.flush();
  return Result;
}

static FrontendInputsAndOutputs resolveSymbolicLinksInInputs(
    FrontendInputsAndOutputs &inputsAndOutputs, StringRef UnresolvedPrimaryFile,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
    std::string &Error) {
  assert(FileSystem);

  toolchain::SmallString<128> PrimaryFile;
  if (auto err = FileSystem->getRealPath(UnresolvedPrimaryFile, PrimaryFile))
    PrimaryFile = UnresolvedPrimaryFile;

  unsigned primaryCount = 0;
  // FIXME: The frontend should be dealing with symlinks, maybe similar to
  // clang's FileManager ?
  FrontendInputsAndOutputs replacementInputsAndOutputs;
  for (const InputFile &input : inputsAndOutputs.getAllInputs()) {
    toolchain::SmallString<128> newFilename;
    if (auto err = FileSystem->getRealPath(input.getFileName(), newFilename))
      newFilename = input.getFileName();
    toolchain::sys::path::native(newFilename);
    bool newIsPrimary = input.isPrimary() ||
                        (!PrimaryFile.empty() && PrimaryFile == newFilename);
    if (newIsPrimary) {
      ++primaryCount;
    }
    assert(primaryCount < 2 && "cannot handle multiple primaries");

    replacementInputsAndOutputs.addInput(
        InputFile(newFilename.str(), newIsPrimary, input.getBuffer()));
  }

  if (PrimaryFile.empty() || primaryCount == 1) {
    return replacementInputsAndOutputs;
  }

  toolchain::SmallString<64> Err;
  toolchain::raw_svector_ostream OS(Err);
  OS << "'" << PrimaryFile << "' is not part of the input files";
  Error = std::string(OS.str());
  return replacementInputsAndOutputs;
}

namespace {
class StreamDiagConsumer : public DiagnosticConsumer {
  toolchain::raw_ostream &OS;

public:
  StreamDiagConsumer(toolchain::raw_ostream &OS) : OS(OS) {}

  void handleDiagnostic(SourceManager &SM,
                        const DiagnosticInfo &Info) override {
    // FIXME: Print location info if available.
    switch (Info.Kind) {
    case DiagnosticKind::Error:
      OS << "error: ";
      break;
    case DiagnosticKind::Warning:
      OS << "warning: ";
      break;
    case DiagnosticKind::Note:
      OS << "note: ";
      break;
    case DiagnosticKind::Remark:
      OS << "remark: ";
      break;
    }
    DiagnosticEngine::formatDiagnosticText(OS, Info.FormatString,
                                           Info.FormatArgs);
  }
};
} // end anonymous namespace

bool ide::initCompilerInvocation(
    CompilerInvocation &Invocation, ArrayRef<const char *> OrigArgs,
    FrontendOptions::ActionType Action, DiagnosticEngine &Diags,
    StringRef UnresolvedPrimaryFile,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
    const std::string &languageExecutablePath,
    const std::string &runtimeResourcePath, time_t sessionTimestamp,
    std::string &Error) {
  SmallVector<const char *, 16> Args;
  // Make sure to put '-resource-dir' at the top to allow overriding them with
  // the passed in arguments.
  Args.push_back("-resource-dir");
  Args.push_back(runtimeResourcePath.c_str());
  Args.append(OrigArgs.begin(), OrigArgs.end());

  SmallString<32> ErrStr;
  toolchain::raw_svector_ostream ErrOS(ErrStr);
  StreamDiagConsumer DiagConsumer(ErrOS);
  Diags.addConsumer(DiagConsumer);

  // Derive 'languagec' path from 'language-frontend' path (languageExecutablePath).
  SmallString<256> driverPath(languageExecutablePath);
  toolchain::sys::path::remove_filename(driverPath);
  toolchain::sys::path::append(driverPath, "languagec");

  bool InvocationCreationFailed =
      driver::getSingleFrontendInvocationFromDriverArguments(
          driverPath, Args, Diags,
          [&](ArrayRef<const char *> FrontendArgs) {
            return Invocation.parseArgs(
                FrontendArgs, Diags, /*ConfigurationFileBuffers=*/nullptr,
                /*workingDirectory=*/"", languageExecutablePath);
          },
          /*ForceNoOutputs=*/true);

  // Remove the StreamDiagConsumer as it's no longer needed.
  Diags.removeConsumer(DiagConsumer);

  Error = std::string(ErrOS.str());
  if (InvocationCreationFailed) {
    return true;
  }

  std::string SymlinkResolveError;
  Invocation.getFrontendOptions().InputsAndOutputs =
      resolveSymbolicLinksInInputs(
          Invocation.getFrontendOptions().InputsAndOutputs,
          UnresolvedPrimaryFile, FileSystem, SymlinkResolveError);

  // SourceKit functionalities want to proceed even if there are missing inputs.
  Invocation.getFrontendOptions()
      .InputsAndOutputs.setShouldRecoverMissingInputs();

  if (!SymlinkResolveError.empty()) {
    // resolveSymbolicLinksInInputs fails if the unresolved primary file is not
    // in the input files. We can't recover from that.
    Error += SymlinkResolveError;
    return true;
  }

  ClangImporterOptions &ImporterOpts = Invocation.getClangImporterOptions();
  ImporterOpts.DetailedPreprocessingRecord = true;

  assert(!Invocation.getModuleName().empty());

  auto &LangOpts = Invocation.getLangOptions();
  LangOpts.AttachCommentsToDecls = true;
  LangOpts.CollectParsedToken = true;
  #if defined(_WIN32)
  // Source files that might be open in an editor should not be memory mapped on Windows,
  // as they will become read-only.
  LangOpts.OpenSourcesAsVolatile = true;
  #endif
  if (LangOpts.PlaygroundTransform) {
    // The playground instrumenter changes the AST in ways that disrupt the
    // SourceKit functionality. Since we don't need the instrumenter, and all we
    // actually need is the playground semantics visible to the user, like
    // silencing the "expression resolves to an unused l-value" error, disable
    // it.
    LangOpts.PlaygroundTransform = false;
  }

  // Disable the index-store functionality for the sourcekitd requests.
  auto &FrontendOpts = Invocation.getFrontendOptions();
  FrontendOpts.IndexStorePath.clear();
  ImporterOpts.IndexStorePath.clear();

  FrontendOpts.RequestedAction = Action;

  // We don't care about LLVMArgs
  FrontendOpts.LLVMArgs.clear();

  // To save the time for module validation, consider the lifetime of ASTManager
  // as a single build session.
  // NOTE: Do this only if '-disable-modules-validate-system-headers' is *not*
  //       explicitly enabled.
  auto &SearchPathOpts = Invocation.getSearchPathOptions();
  if (!SearchPathOpts.DisableModulesValidateSystemDependencies) {
    // NOTE: 'SessionTimestamp - 1' because clang compares it with '<=' that may
    //       cause unnecessary validations if they happens within one second
    //       from the SourceKit startup.
    ImporterOpts.ExtraArgs.push_back("-fbuild-session-timestamp=" +
                                     std::to_string(sessionTimestamp - 1));
    ImporterOpts.ExtraArgs.push_back(
        "-fmodules-validate-once-per-build-session");
  }

  // Disable expensive SIL options to reduce time spent in SILGen.
  disableExpensiveSILOptions(Invocation.getSILOptions());

  return false;
}

bool ide::initInvocationByClangArguments(ArrayRef<const char *> ArgList,
                                         CompilerInvocation &Invok,
                                         std::string &Error) {
  toolchain::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts{
    new clang::DiagnosticOptions()
  };

  clang::TextDiagnosticBuffer DiagBuf;
  toolchain::IntrusiveRefCntPtr<clang::DiagnosticsEngine> ClangDiags =
      clang::CompilerInstance::createDiagnostics(DiagOpts.get(), &DiagBuf,
                                                 /*ShouldOwnClient=*/false);

  // Clang expects this to be like an actual command line. So we need to pass in
  // "clang" for argv[0].
  std::vector<const char *> ClangArgList;
  ClangArgList.push_back("clang");
  ClangArgList.insert(ClangArgList.end(), ArgList.begin(), ArgList.end());

  // Create a new Clang compiler invocation.
  clang::CreateInvocationOptions CIOpts;
  CIOpts.Diags = ClangDiags;
  CIOpts.ProbePrecompiled = true;
  std::unique_ptr<clang::CompilerInvocation> ClangInvok =
      clang::createInvocation(ClangArgList, std::move(CIOpts));
  if (!ClangInvok || ClangDiags->hasErrorOccurred()) {
    for (auto I = DiagBuf.err_begin(), E = DiagBuf.err_end(); I != E; ++I) {
      Error += I->second;
      Error += " ";
    }
    return true;
  }

  auto &PPOpts = ClangInvok->getPreprocessorOpts();
  auto &HSOpts = ClangInvok->getHeaderSearchOpts();

  Invok.setTargetTriple(adjustClangTriple(ClangInvok->getTargetOpts().Triple));
  if (!HSOpts.Sysroot.empty())
    Invok.setSDKPath(HSOpts.Sysroot);
  if (!HSOpts.ModuleCachePath.empty())
    Invok.setClangModuleCachePath(HSOpts.ModuleCachePath);

  auto &CCArgs = Invok.getClangImporterOptions().ExtraArgs;
  for (auto MacroEntry : PPOpts.Macros) {
    std::string MacroFlag;
    if (MacroEntry.second)
      MacroFlag += "-U";
    else
      MacroFlag += "-D";
    MacroFlag += MacroEntry.first;
    CCArgs.push_back(MacroFlag);
  }

  for (auto &Entry : HSOpts.UserEntries) {
    switch (Entry.Group) {
    case clang::frontend::Quoted:
      CCArgs.push_back("-iquote");
      CCArgs.push_back(Entry.Path);
      break;
    case clang::frontend::Angled: {
      std::string Flag;
      if (Entry.IsFramework)
        Flag += "-F";
      else
        Flag += "-I";
      Flag += Entry.Path;
      CCArgs.push_back(Flag);
      break;
    }
    case clang::frontend::System:
      if (Entry.IsFramework)
        CCArgs.push_back("-iframework");
      else
        CCArgs.push_back("-isystem");
      CCArgs.push_back(Entry.Path);
      break;
    case clang::frontend::ExternCSystem:
    case clang::frontend::CSystem:
    case clang::frontend::CXXSystem:
    case clang::frontend::ObjCSystem:
    case clang::frontend::ObjCXXSystem:
    case clang::frontend::After:
      break;
    }
  }

  if (!PPOpts.ImplicitPCHInclude.empty()) {
    clang::FileSystemOptions FileSysOpts;
    clang::FileManager FileMgr(FileSysOpts);
    auto PCHContainerOperations =
        std::make_shared<clang::PCHContainerOperations>();
    std::string HeaderFile = clang::ASTReader::getOriginalSourceFile(
        PPOpts.ImplicitPCHInclude, FileMgr,
        PCHContainerOperations->getRawReader(), *ClangDiags);
    if (!HeaderFile.empty()) {
      CCArgs.push_back("-include");
      CCArgs.push_back(std::move(HeaderFile));
    }
  }
  for (auto &Header : PPOpts.Includes) {
    CCArgs.push_back("-include");
    CCArgs.push_back(Header);
  }

  for (auto &Entry : HSOpts.ModulesIgnoreMacros) {
    std::string Flag = "-fmodules-ignore-macro=";
    Flag += Entry;
    CCArgs.push_back(Flag);
  }

  for (auto &Entry : HSOpts.VFSOverlayFiles) {
    CCArgs.push_back("-ivfsoverlay");
    CCArgs.push_back(Entry);
  }

  if (!ClangInvok->getLangOpts().isCompilingModule()) {
    CCArgs.push_back("-Xclang");
    toolchain::SmallString<64> Str;
    Str += "-fmodule-name=";
    Str += ClangInvok->getLangOpts().CurrentModule;
    CCArgs.push_back(std::string(Str.str()));
  }

  if (PPOpts.DetailedRecord) {
    Invok.getClangImporterOptions().DetailedPreprocessingRecord = true;
  }

  if (!ClangInvok->getFrontendOpts().Inputs.empty()) {
    Invok.getFrontendOptions().ImplicitObjCHeaderPath =
        ClangInvok->getFrontendOpts().Inputs[0].getFile().str();
  }

  return false;
}
