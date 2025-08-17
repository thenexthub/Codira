//===-- HeaderIncludeGen.cpp - Generate Header Includes -------------------===//
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

#include "language/Core/Frontend/DependencyOutputOptions.h"
#include "language/Core/Frontend/Utils.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Lex/Preprocessor.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/JSON.h"
#include "toolchain/Support/raw_ostream.h"
using namespace language::Core;

namespace {
class HeaderIncludesCallback : public PPCallbacks {
  SourceManager &SM;
  raw_ostream *OutputFile;
  const DependencyOutputOptions &DepOpts;
  unsigned CurrentIncludeDepth;
  bool HasProcessedPredefines;
  bool OwnsOutputFile;
  bool ShowAllHeaders;
  bool ShowDepth;
  bool MSStyle;

public:
  HeaderIncludesCallback(const Preprocessor *PP, bool ShowAllHeaders_,
                         raw_ostream *OutputFile_,
                         const DependencyOutputOptions &DepOpts,
                         bool OwnsOutputFile_, bool ShowDepth_, bool MSStyle_)
      : SM(PP->getSourceManager()), OutputFile(OutputFile_), DepOpts(DepOpts),
        CurrentIncludeDepth(0), HasProcessedPredefines(false),
        OwnsOutputFile(OwnsOutputFile_), ShowAllHeaders(ShowAllHeaders_),
        ShowDepth(ShowDepth_), MSStyle(MSStyle_) {}

  ~HeaderIncludesCallback() override {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  HeaderIncludesCallback(const HeaderIncludesCallback &) = delete;
  HeaderIncludesCallback &operator=(const HeaderIncludesCallback &) = delete;

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override;

  void FileSkipped(const FileEntryRef &SkippedFile, const Token &FilenameTok,
                   SrcMgr::CharacteristicKind FileType) override;

private:
  bool ShouldShowHeader(SrcMgr::CharacteristicKind HeaderType) {
    if (!DepOpts.IncludeSystemHeaders && isSystem(HeaderType))
      return false;

    // Show the current header if we are (a) past the predefines, or (b) showing
    // all headers and in the predefines at a depth past the initial file and
    // command line buffers.
    return (HasProcessedPredefines ||
            (ShowAllHeaders && CurrentIncludeDepth > 2));
  }
};

/// A callback for emitting header usage information to a file in JSON. Each
/// line in the file is a JSON object that includes the source file name and
/// the list of headers directly or indirectly included from it. For example:
///
/// {"source":"/tmp/foo.c",
///  "includes":["/usr/include/stdio.h", "/usr/include/stdlib.h"]}
///
/// To reduce the amount of data written to the file, we only record system
/// headers that are directly included from a file that isn't in the system
/// directory.
class HeaderIncludesJSONCallback : public PPCallbacks {
  SourceManager &SM;
  raw_ostream *OutputFile;
  bool OwnsOutputFile;
  SmallVector<std::string, 16> IncludedHeaders;

public:
  HeaderIncludesJSONCallback(const Preprocessor *PP, raw_ostream *OutputFile_,
                             bool OwnsOutputFile_)
      : SM(PP->getSourceManager()), OutputFile(OutputFile_),
        OwnsOutputFile(OwnsOutputFile_) {}

  ~HeaderIncludesJSONCallback() override {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  HeaderIncludesJSONCallback(const HeaderIncludesJSONCallback &) = delete;
  HeaderIncludesJSONCallback &
  operator=(const HeaderIncludesJSONCallback &) = delete;

  void EndOfMainFile() override;

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override;

  void FileSkipped(const FileEntryRef &SkippedFile, const Token &FilenameTok,
                   SrcMgr::CharacteristicKind FileType) override;
};

/// A callback for emitting direct header and module usage information to a
/// file in JSON. The output format is like HeaderIncludesJSONCallback but has
/// an array of separate entries, one for each non-system source file used in
/// the compilation showing only the direct includes and imports from that file.
class HeaderIncludesDirectPerFileCallback : public PPCallbacks {
  SourceManager &SM;
  HeaderSearch &HSI;
  raw_ostream *OutputFile;
  bool OwnsOutputFile;
  using DependencyMap = toolchain::DenseMap<FileEntryRef, SmallVector<FileEntryRef>>;
  DependencyMap Dependencies;

public:
  HeaderIncludesDirectPerFileCallback(const Preprocessor *PP,
                                      raw_ostream *OutputFile_,
                                      bool OwnsOutputFile_)
      : SM(PP->getSourceManager()), HSI(PP->getHeaderSearchInfo()),
        OutputFile(OutputFile_), OwnsOutputFile(OwnsOutputFile_) {}

  ~HeaderIncludesDirectPerFileCallback() override {
    if (OwnsOutputFile)
      delete OutputFile;
  }

  HeaderIncludesDirectPerFileCallback(
      const HeaderIncludesDirectPerFileCallback &) = delete;
  HeaderIncludesDirectPerFileCallback &
  operator=(const HeaderIncludesDirectPerFileCallback &) = delete;

  void EndOfMainFile() override;

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override;

  void moduleImport(SourceLocation ImportLoc, ModuleIdPath Path,
                    const Module *Imported) override;
};
}

static void PrintHeaderInfo(raw_ostream *OutputFile, StringRef Filename,
                            bool ShowDepth, unsigned CurrentIncludeDepth,
                            bool MSStyle) {
  // Write to a temporary string to avoid unnecessary flushing on errs().
  SmallString<512> Pathname(Filename);
  if (!MSStyle)
    Lexer::Stringify(Pathname);

  SmallString<256> Msg;
  if (MSStyle)
    Msg += "Note: including file:";

  if (ShowDepth) {
    // The main source file is at depth 1, so skip one dot.
    for (unsigned i = 1; i != CurrentIncludeDepth; ++i)
      Msg += MSStyle ? ' ' : '.';

    if (!MSStyle)
      Msg += ' ';
  }
  Msg += Pathname;
  Msg += '\n';

  *OutputFile << Msg;
  OutputFile->flush();
}

void language::Core::AttachHeaderIncludeGen(Preprocessor &PP,
                                   const DependencyOutputOptions &DepOpts,
                                   bool ShowAllHeaders, StringRef OutputPath,
                                   bool ShowDepth, bool MSStyle) {
  raw_ostream *OutputFile = &toolchain::errs();
  bool OwnsOutputFile = false;

  // Choose output stream, when printing in cl.exe /showIncludes style.
  if (MSStyle) {
    switch (DepOpts.ShowIncludesDest) {
    default:
      toolchain_unreachable("Invalid destination for /showIncludes output!");
    case ShowIncludesDestination::Stderr:
      OutputFile = &toolchain::errs();
      break;
    case ShowIncludesDestination::Stdout:
      OutputFile = &toolchain::outs();
      break;
    }
  }

  // Open the output file, if used.
  if (!OutputPath.empty()) {
    std::error_code EC;
    toolchain::raw_fd_ostream *OS = new toolchain::raw_fd_ostream(
        OutputPath.str(), EC,
        toolchain::sys::fs::OF_Append | toolchain::sys::fs::OF_TextWithCRLF);
    if (EC) {
      PP.getDiagnostics().Report(language::Core::diag::warn_fe_cc_print_header_failure)
          << EC.message();
      delete OS;
    } else {
      OS->SetUnbuffered();
      OutputFile = OS;
      OwnsOutputFile = true;
    }
  }

  switch (DepOpts.HeaderIncludeFormat) {
  case HIFMT_None:
    toolchain_unreachable("unexpected header format kind");
  case HIFMT_Textual: {
    assert(DepOpts.HeaderIncludeFiltering == HIFIL_None &&
           "header filtering is currently always disabled when output format is"
           "textual");
    // Print header info for extra headers, pretending they were discovered by
    // the regular preprocessor. The primary use case is to support proper
    // generation of Make / Ninja file dependencies for implicit includes, such
    // as sanitizer ignorelists. It's only important for cl.exe compatibility,
    // the GNU way to generate rules is -M / -MM / -MD / -MMD.
    for (const auto &Header : DepOpts.ExtraDeps)
      PrintHeaderInfo(OutputFile, Header.first, ShowDepth, 2, MSStyle);
    PP.addPPCallbacks(std::make_unique<HeaderIncludesCallback>(
        &PP, ShowAllHeaders, OutputFile, DepOpts, OwnsOutputFile, ShowDepth,
        MSStyle));
    break;
  }
  case HIFMT_JSON:
    switch (DepOpts.HeaderIncludeFiltering) {
    default:
      toolchain_unreachable("Unknown HeaderIncludeFilteringKind enum");
    case HIFIL_Only_Direct_System:
      PP.addPPCallbacks(std::make_unique<HeaderIncludesJSONCallback>(
          &PP, OutputFile, OwnsOutputFile));
      break;
    case HIFIL_Direct_Per_File:
      PP.addPPCallbacks(std::make_unique<HeaderIncludesDirectPerFileCallback>(
          &PP, OutputFile, OwnsOutputFile));
      break;
    }
    break;
  }
}

void HeaderIncludesCallback::FileChanged(SourceLocation Loc,
                                         FileChangeReason Reason,
                                         SrcMgr::CharacteristicKind NewFileType,
                                         FileID PrevFID) {
  // Unless we are exiting a #include, make sure to skip ahead to the line the
  // #include directive was at.
  PresumedLoc UserLoc = SM.getPresumedLoc(Loc);
  if (UserLoc.isInvalid())
    return;

  // Adjust the current include depth.
  if (Reason == PPCallbacks::EnterFile) {
    ++CurrentIncludeDepth;
  } else if (Reason == PPCallbacks::ExitFile) {
    if (CurrentIncludeDepth)
      --CurrentIncludeDepth;

    // We track when we are done with the predefines by watching for the first
    // place where we drop back to a nesting depth of 1.
    if (CurrentIncludeDepth == 1 && !HasProcessedPredefines)
      HasProcessedPredefines = true;

    return;
  } else {
    return;
  }

  if (!ShouldShowHeader(NewFileType))
    return;

  unsigned IncludeDepth = CurrentIncludeDepth;
  if (!HasProcessedPredefines)
    --IncludeDepth; // Ignore indent from <built-in>.

  // FIXME: Identify headers in a more robust way than comparing their name to
  // "<command line>" and "<built-in>" in a bunch of places.
  if (Reason == PPCallbacks::EnterFile &&
      UserLoc.getFilename() != StringRef("<command line>")) {
    PrintHeaderInfo(OutputFile, UserLoc.getFilename(), ShowDepth, IncludeDepth,
                    MSStyle);
  }
}

void HeaderIncludesCallback::FileSkipped(const FileEntryRef &SkippedFile, const
                                         Token &FilenameTok,
                                         SrcMgr::CharacteristicKind FileType) {
  if (!DepOpts.ShowSkippedHeaderIncludes)
    return;

  if (!ShouldShowHeader(FileType))
    return;

  PrintHeaderInfo(OutputFile, SkippedFile.getName(), ShowDepth,
                  CurrentIncludeDepth + 1, MSStyle);
}

void HeaderIncludesJSONCallback::EndOfMainFile() {
  OptionalFileEntryRef FE = SM.getFileEntryRefForID(SM.getMainFileID());
  SmallString<256> MainFile;
  if (FE) {
    MainFile += FE->getName();
    SM.getFileManager().makeAbsolutePath(MainFile);
  }

  std::string Str;
  toolchain::raw_string_ostream OS(Str);
  toolchain::json::OStream JOS(OS);
  JOS.object([&] {
    JOS.attribute("source", MainFile.c_str());
    JOS.attributeArray("includes", [&] {
      toolchain::StringSet<> SeenHeaders;
      for (const std::string &H : IncludedHeaders)
        if (SeenHeaders.insert(H).second)
          JOS.value(H);
    });
  });
  OS << "\n";

  if (OutputFile->get_kind() == raw_ostream::OStreamKind::OK_FDStream) {
    toolchain::raw_fd_ostream *FDS = static_cast<toolchain::raw_fd_ostream *>(OutputFile);
    if (auto L = FDS->lock())
      *OutputFile << Str;
  } else
    *OutputFile << Str;
}

/// Determine whether the header file should be recorded. The header file should
/// be recorded only if the header file is a system header and the current file
/// isn't a system header.
static bool shouldRecordNewFile(SrcMgr::CharacteristicKind NewFileType,
                                SourceLocation PrevLoc, SourceManager &SM) {
  return SrcMgr::isSystem(NewFileType) && !SM.isInSystemHeader(PrevLoc);
}

void HeaderIncludesJSONCallback::FileChanged(
    SourceLocation Loc, FileChangeReason Reason,
    SrcMgr::CharacteristicKind NewFileType, FileID PrevFID) {
  if (PrevFID.isInvalid() ||
      !shouldRecordNewFile(NewFileType, SM.getLocForStartOfFile(PrevFID), SM))
    return;

  // Unless we are exiting a #include, make sure to skip ahead to the line the
  // #include directive was at.
  PresumedLoc UserLoc = SM.getPresumedLoc(Loc);
  if (UserLoc.isInvalid())
    return;

  if (Reason == PPCallbacks::EnterFile &&
      UserLoc.getFilename() != StringRef("<command line>"))
    IncludedHeaders.push_back(UserLoc.getFilename());
}

void HeaderIncludesJSONCallback::FileSkipped(
    const FileEntryRef &SkippedFile, const Token &FilenameTok,
    SrcMgr::CharacteristicKind FileType) {
  if (!shouldRecordNewFile(FileType, FilenameTok.getLocation(), SM))
    return;

  IncludedHeaders.push_back(SkippedFile.getName().str());
}

void HeaderIncludesDirectPerFileCallback::EndOfMainFile() {
  if (Dependencies.empty())
    return;

  // Sort the files so that the output does not depend on the DenseMap order.
  SmallVector<FileEntryRef> SourceFiles;
  for (auto F = Dependencies.begin(), FEnd = Dependencies.end(); F != FEnd;
       ++F) {
    SourceFiles.push_back(F->first);
  }
  toolchain::sort(SourceFiles, [](const FileEntryRef &LHS, const FileEntryRef &RHS) {
    return LHS.getUID() < RHS.getUID();
  });

  std::string Str;
  toolchain::raw_string_ostream OS(Str);
  toolchain::json::OStream JOS(OS);
  JOS.array([&] {
    for (auto S = SourceFiles.begin(), SE = SourceFiles.end(); S != SE; ++S) {
      JOS.object([&] {
        SmallVector<FileEntryRef> &Deps = Dependencies[*S];
        JOS.attribute("source", S->getName().str());
        JOS.attributeArray("includes", [&] {
          for (unsigned I = 0, N = Deps.size(); I != N; ++I)
            JOS.value(Deps[I].getName().str());
        });
      });
    }
  });
  OS << "\n";

  if (OutputFile->get_kind() == raw_ostream::OStreamKind::OK_FDStream) {
    toolchain::raw_fd_ostream *FDS = static_cast<toolchain::raw_fd_ostream *>(OutputFile);
    if (auto L = FDS->lock())
      *OutputFile << Str;
  } else
    *OutputFile << Str;
}

void HeaderIncludesDirectPerFileCallback::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, OptionalFileEntryRef File,
    StringRef SearchPath, StringRef RelativePath, const Module *SuggestedModule,
    bool ModuleImported, SrcMgr::CharacteristicKind FileType) {
  if (!File)
    return;

  SourceLocation Loc = SM.getExpansionLoc(HashLoc);
  if (SM.isInSystemHeader(Loc))
    return;
  OptionalFileEntryRef FromFile = SM.getFileEntryRefForID(SM.getFileID(Loc));
  if (!FromFile)
    return;

  Dependencies[*FromFile].push_back(*File);
}

void HeaderIncludesDirectPerFileCallback::moduleImport(SourceLocation ImportLoc,
                                                       ModuleIdPath Path,
                                                       const Module *Imported) {
  if (!Imported)
    return;

  SourceLocation Loc = SM.getExpansionLoc(ImportLoc);
  if (SM.isInSystemHeader(Loc))
    return;
  OptionalFileEntryRef FromFile = SM.getFileEntryRefForID(SM.getFileID(Loc));
  if (!FromFile)
    return;

  OptionalFileEntryRef ModuleMapFile =
      HSI.getModuleMap().getModuleMapFileForUniquing(Imported);
  if (!ModuleMapFile)
    return;

  Dependencies[*FromFile].push_back(*ModuleMapFile);
}
