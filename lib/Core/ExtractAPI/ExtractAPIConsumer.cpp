//===- ExtractAPI/ExtractAPIConsumer.cpp ------------------------*- C++ -*-===//
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
///
/// \file
/// This file implements the ExtractAPIAction, and ASTConsumer to collect API
/// information.
///
//===----------------------------------------------------------------------===//

#include "language/Core/AST/ASTConcept.h"
#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "language/Core/Basic/FileEntry.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/ExtractAPI/API.h"
#include "language/Core/ExtractAPI/APIIgnoresList.h"
#include "language/Core/ExtractAPI/ExtractAPIVisitor.h"
#include "language/Core/ExtractAPI/FrontendActions.h"
#include "language/Core/ExtractAPI/Serialization/SymbolGraphSerializer.h"
#include "language/Core/Frontend/ASTConsumers.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/FrontendOptions.h"
#include "language/Core/Frontend/MultiplexConsumer.h"
#include "language/Core/Index/USRGeneration.h"
#include "language/Core/InstallAPI/HeaderFile.h"
#include "language/Core/Lex/MacroInfo.h"
#include "language/Core/Lex/PPCallbacks.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Casting.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Regex.h"
#include "toolchain/Support/raw_ostream.h"
#include <memory>
#include <optional>
#include <utility>

using namespace language::Core;
using namespace extractapi;

namespace {

std::optional<std::string> getRelativeIncludeName(const CompilerInstance &CI,
                                                  StringRef File,
                                                  bool *IsQuoted = nullptr) {
  assert(CI.hasFileManager() &&
         "CompilerInstance does not have a FileNamager!");

  using namespace toolchain::sys;
  const auto &FS = CI.getVirtualFileSystem();

  SmallString<128> FilePath(File.begin(), File.end());
  FS.makeAbsolute(FilePath);
  path::remove_dots(FilePath, true);
  FilePath = path::convert_to_slash(FilePath);
  File = FilePath;

  // Checks whether `Dir` is a strict path prefix of `File`. If so returns
  // the prefix length. Otherwise return 0.
  auto CheckDir = [&](toolchain::StringRef Dir) -> unsigned {
    toolchain::SmallString<32> DirPath(Dir.begin(), Dir.end());
    FS.makeAbsolute(DirPath);
    path::remove_dots(DirPath, true);
    Dir = DirPath;
    for (auto NI = path::begin(File), NE = path::end(File),
              DI = path::begin(Dir), DE = path::end(Dir);
         /*termination condition in loop*/; ++NI, ++DI) {
      // '.' components in File are ignored.
      while (NI != NE && *NI == ".")
        ++NI;
      if (NI == NE)
        break;

      // '.' components in Dir are ignored.
      while (DI != DE && *DI == ".")
        ++DI;

      // Dir is a prefix of File, up to '.' components and choice of path
      // separators.
      if (DI == DE)
        return NI - path::begin(File);

      // Consider all path separators equal.
      if (NI->size() == 1 && DI->size() == 1 &&
          path::is_separator(NI->front()) && path::is_separator(DI->front()))
        continue;

      // Special case Apple .sdk folders since the search path is typically a
      // symlink like `iPhoneSimulator14.5.sdk` while the file is instead
      // located in `iPhoneSimulator.sdk` (the real folder).
      if (NI->ends_with(".sdk") && DI->ends_with(".sdk")) {
        StringRef NBasename = path::stem(*NI);
        StringRef DBasename = path::stem(*DI);
        if (DBasename.starts_with(NBasename))
          continue;
      }

      if (*NI != *DI)
        break;
    }
    return 0;
  };

  unsigned PrefixLength = 0;

  // Go through the search paths and find the first one that is a prefix of
  // the header.
  for (const auto &Entry : CI.getHeaderSearchOpts().UserEntries) {
    // Note whether the match is found in a quoted entry.
    if (IsQuoted)
      *IsQuoted = Entry.Group == frontend::Quoted;

    if (auto EntryFile = CI.getFileManager().getOptionalFileRef(Entry.Path)) {
      if (auto HMap = HeaderMap::Create(*EntryFile, CI.getFileManager())) {
        // If this is a headermap entry, try to reverse lookup the full path
        // for a spelled name before mapping.
        StringRef SpelledFilename = HMap->reverseLookupFilename(File);
        if (!SpelledFilename.empty())
          return SpelledFilename.str();

        // No matching mapping in this headermap, try next search entry.
        continue;
      }
    }

    // Entry is a directory search entry, try to check if it's a prefix of File.
    PrefixLength = CheckDir(Entry.Path);
    if (PrefixLength > 0) {
      // The header is found in a framework path, construct the framework-style
      // include name `<Framework/Header.h>`
      if (Entry.IsFramework) {
        SmallVector<StringRef, 4> Matches;
        language::Core::installapi::HeaderFile::getFrameworkIncludeRule().match(
            File, &Matches);
        // Returned matches are always in stable order.
        if (Matches.size() != 4)
          return std::nullopt;

        return path::convert_to_slash(
            (Matches[1].drop_front(Matches[1].rfind('/') + 1) + "/" +
             Matches[3])
                .str());
      }

      // The header is found in a normal search path, strip the search path
      // prefix to get an include name.
      return path::convert_to_slash(File.drop_front(PrefixLength));
    }
  }

  // Couldn't determine a include name, use full path instead.
  return std::nullopt;
}

std::optional<std::string> getRelativeIncludeName(const CompilerInstance &CI,
                                                  FileEntryRef FE,
                                                  bool *IsQuoted = nullptr) {
  return getRelativeIncludeName(CI, FE.getNameAsRequested(), IsQuoted);
}

struct LocationFileChecker {
  bool operator()(SourceLocation Loc) {
    // If the loc refers to a macro expansion we need to first get the file
    // location of the expansion.
    auto &SM = CI.getSourceManager();
    auto FileLoc = SM.getFileLoc(Loc);
    FileID FID = SM.getFileID(FileLoc);
    if (FID.isInvalid())
      return false;

    OptionalFileEntryRef File = SM.getFileEntryRefForID(FID);
    if (!File)
      return false;

    if (KnownFileEntries.count(*File))
      return true;

    if (ExternalFileEntries.count(*File))
      return false;

    // Try to reduce the include name the same way we tried to include it.
    bool IsQuoted = false;
    if (auto IncludeName = getRelativeIncludeName(CI, *File, &IsQuoted))
      if (toolchain::any_of(KnownFiles,
                       [&IsQuoted, &IncludeName](const auto &KnownFile) {
                         return KnownFile.first.equals(*IncludeName) &&
                                KnownFile.second == IsQuoted;
                       })) {
        KnownFileEntries.insert(*File);
        return true;
      }

    // Record that the file was not found to avoid future reverse lookup for
    // the same file.
    ExternalFileEntries.insert(*File);
    return false;
  }

  LocationFileChecker(const CompilerInstance &CI,
                      SmallVector<std::pair<SmallString<32>, bool>> &KnownFiles)
      : CI(CI), KnownFiles(KnownFiles), ExternalFileEntries() {
    for (const auto &KnownFile : KnownFiles)
      if (auto FE = CI.getFileManager().getOptionalFileRef(KnownFile.first))
        KnownFileEntries.insert(*FE);
  }

private:
  const CompilerInstance &CI;
  SmallVector<std::pair<SmallString<32>, bool>> &KnownFiles;
  toolchain::DenseSet<const FileEntry *> KnownFileEntries;
  toolchain::DenseSet<const FileEntry *> ExternalFileEntries;
};

struct BatchExtractAPIVisitor : ExtractAPIVisitor<BatchExtractAPIVisitor> {
  bool shouldDeclBeIncluded(const Decl *D) const {
    bool ShouldBeIncluded = true;
    // Check that we have the definition for redeclarable types.
    if (auto *TD = toolchain::dyn_cast<TagDecl>(D))
      ShouldBeIncluded = TD->isThisDeclarationADefinition();
    else if (auto *Interface = toolchain::dyn_cast<ObjCInterfaceDecl>(D))
      ShouldBeIncluded = Interface->isThisDeclarationADefinition();
    else if (auto *Protocol = toolchain::dyn_cast<ObjCProtocolDecl>(D))
      ShouldBeIncluded = Protocol->isThisDeclarationADefinition();

    ShouldBeIncluded = ShouldBeIncluded && LCF(D->getLocation());
    return ShouldBeIncluded;
  }

  BatchExtractAPIVisitor(LocationFileChecker &LCF, ASTContext &Context,
                         APISet &API)
      : ExtractAPIVisitor<BatchExtractAPIVisitor>(Context, API), LCF(LCF) {}

private:
  LocationFileChecker &LCF;
};

class WrappingExtractAPIConsumer : public ASTConsumer {
public:
  WrappingExtractAPIConsumer(ASTContext &Context, APISet &API)
      : Visitor(Context, API) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    // Use ExtractAPIVisitor to traverse symbol declarations in the context.
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  ExtractAPIVisitor<> Visitor;
};

class ExtractAPIConsumer : public ASTConsumer {
public:
  ExtractAPIConsumer(ASTContext &Context,
                     std::unique_ptr<LocationFileChecker> LCF, APISet &API)
      : Visitor(*LCF, Context, API), LCF(std::move(LCF)) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    // Use ExtractAPIVisitor to traverse symbol declarations in the context.
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  BatchExtractAPIVisitor Visitor;
  std::unique_ptr<LocationFileChecker> LCF;
};

class MacroCallback : public PPCallbacks {
public:
  MacroCallback(const SourceManager &SM, APISet &API, Preprocessor &PP)
      : SM(SM), API(API), PP(PP) {}

  void EndOfMainFile() override {
    for (const auto &M : PP.macros()) {
      auto *II = M.getFirst();
      auto MD = PP.getMacroDefinition(II);
      auto *MI = MD.getMacroInfo();

      if (!MI)
        continue;

      // Ignore header guard macros
      if (MI->isUsedForHeaderGuard())
        continue;

      // Ignore builtin macros and ones defined via the command line.
      if (MI->isBuiltinMacro())
        continue;

      auto DefLoc = MI->getDefinitionLoc();

      if (SM.isInPredefinedFile(DefLoc))
        continue;

      auto AssociatedModuleMacros = MD.getModuleMacros();
      StringRef OwningModuleName;
      if (!AssociatedModuleMacros.empty())
        OwningModuleName = AssociatedModuleMacros.back()
                               ->getOwningModule()
                               ->getTopLevelModuleName();

      if (!shouldMacroBeIncluded(DefLoc, OwningModuleName))
        continue;

      StringRef Name = II->getName();
      PresumedLoc Loc = SM.getPresumedLoc(DefLoc);
      SmallString<128> USR;
      index::generateUSRForMacro(Name, DefLoc, SM, USR);
      API.createRecord<extractapi::MacroDefinitionRecord>(
          USR, Name, SymbolReference(), Loc,
          DeclarationFragmentsBuilder::getFragmentsForMacro(Name, MI),
          DeclarationFragmentsBuilder::getSubHeadingForMacro(Name),
          SM.isInSystemHeader(DefLoc));
    }
  }

  virtual bool shouldMacroBeIncluded(const SourceLocation &MacroLoc,
                                     StringRef ModuleName) {
    return true;
  }

  const SourceManager &SM;
  APISet &API;
  Preprocessor &PP;
};

class APIMacroCallback : public MacroCallback {
public:
  APIMacroCallback(const SourceManager &SM, APISet &API, Preprocessor &PP,
                   LocationFileChecker &LCF)
      : MacroCallback(SM, API, PP), LCF(LCF) {}

  bool shouldMacroBeIncluded(const SourceLocation &MacroLoc,
                             StringRef ModuleName) override {
    // Do not include macros from external files
    return LCF(MacroLoc);
  }

private:
  LocationFileChecker &LCF;
};

std::unique_ptr<toolchain::raw_pwrite_stream>
createAdditionalSymbolGraphFile(CompilerInstance &CI, Twine BaseName) {
  auto OutputDirectory = CI.getFrontendOpts().SymbolGraphOutputDir;

  SmallString<256> FileName;
  toolchain::sys::path::append(FileName, OutputDirectory,
                          BaseName + ".symbols.json");
  return CI.createOutputFile(
      FileName, /*Binary*/ false, /*RemoveFileOnSignal*/ false,
      /*UseTemporary*/ true, /*CreateMissingDirectories*/ true);
}

} // namespace

void ExtractAPIActionBase::ImplEndSourceFileAction(CompilerInstance &CI) {
  SymbolGraphSerializerOption SerializationOptions;
  SerializationOptions.Compact = !CI.getFrontendOpts().EmitPrettySymbolGraphs;
  SerializationOptions.EmitSymbolLabelsForTesting =
      CI.getFrontendOpts().EmitSymbolGraphSymbolLabelsForTesting;

  if (CI.getFrontendOpts().EmitExtensionSymbolGraphs) {
    auto ConstructOutputFile = [&CI](Twine BaseName) {
      return createAdditionalSymbolGraphFile(CI, BaseName);
    };

    SymbolGraphSerializer::serializeWithExtensionGraphs(
        *OS, *API, IgnoresList, ConstructOutputFile, SerializationOptions);
  } else {
    SymbolGraphSerializer::serializeMainSymbolGraph(*OS, *API, IgnoresList,
                                                    SerializationOptions);
  }

  // Flush the stream and close the main output stream.
  OS.reset();
}

std::unique_ptr<ASTConsumer>
ExtractAPIAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  auto ProductName = CI.getFrontendOpts().ProductName;

  if (CI.getFrontendOpts().SymbolGraphOutputDir.empty())
    OS = CI.createDefaultOutputFile(/*Binary*/ false, InFile,
                                    /*Extension*/ "symbols.json",
                                    /*RemoveFileOnSignal*/ false,
                                    /*CreateMissingDirectories*/ true);
  else
    OS = createAdditionalSymbolGraphFile(CI, ProductName);

  if (!OS)
    return nullptr;

  // Now that we have enough information about the language options and the
  // target triple, let's create the APISet before anyone uses it.
  API = std::make_unique<APISet>(
      CI.getTarget().getTriple(),
      CI.getFrontendOpts().Inputs.back().getKind().getLanguage(), ProductName);

  auto LCF = std::make_unique<LocationFileChecker>(CI, KnownInputFiles);

  CI.getPreprocessor().addPPCallbacks(std::make_unique<APIMacroCallback>(
      CI.getSourceManager(), *API, CI.getPreprocessor(), *LCF));

  // Do not include location in anonymous decls.
  PrintingPolicy Policy = CI.getASTContext().getPrintingPolicy();
  Policy.AnonymousTagLocations = false;
  CI.getASTContext().setPrintingPolicy(Policy);

  if (!CI.getFrontendOpts().ExtractAPIIgnoresFileList.empty()) {
    toolchain::handleAllErrors(
        APIIgnoresList::create(CI.getFrontendOpts().ExtractAPIIgnoresFileList,
                               CI.getFileManager())
            .moveInto(IgnoresList),
        [&CI](const IgnoresFileNotFound &Err) {
          CI.getDiagnostics().Report(
              diag::err_extract_api_ignores_file_not_found)
              << Err.Path;
        });
  }

  return std::make_unique<ExtractAPIConsumer>(CI.getASTContext(),
                                              std::move(LCF), *API);
}

bool ExtractAPIAction::PrepareToExecuteAction(CompilerInstance &CI) {
  auto &Inputs = CI.getFrontendOpts().Inputs;
  if (Inputs.empty())
    return true;

  if (!CI.hasFileManager())
    if (!CI.createFileManager())
      return false;

  auto Kind = Inputs[0].getKind();

  // Convert the header file inputs into a single input buffer.
  SmallString<256> HeaderContents;
  bool IsQuoted = false;
  for (const FrontendInputFile &FIF : Inputs) {
    if (Kind.isObjectiveC())
      HeaderContents += "#import";
    else
      HeaderContents += "#include";

    StringRef FilePath = FIF.getFile();
    if (auto RelativeName = getRelativeIncludeName(CI, FilePath, &IsQuoted)) {
      if (IsQuoted)
        HeaderContents += " \"";
      else
        HeaderContents += " <";

      HeaderContents += *RelativeName;

      if (IsQuoted)
        HeaderContents += "\"\n";
      else
        HeaderContents += ">\n";
      KnownInputFiles.emplace_back(static_cast<SmallString<32>>(*RelativeName),
                                   IsQuoted);
    } else {
      HeaderContents += " \"";
      HeaderContents += FilePath;
      HeaderContents += "\"\n";
      KnownInputFiles.emplace_back(FilePath, true);
    }
  }

  if (CI.getHeaderSearchOpts().Verbose)
    CI.getVerboseOutputStream() << getInputBufferName() << ":\n"
                                << HeaderContents << "\n";

  Buffer = toolchain::MemoryBuffer::getMemBufferCopy(HeaderContents,
                                                getInputBufferName());

  // Set that buffer up as our "real" input in the CompilerInstance.
  Inputs.clear();
  Inputs.emplace_back(Buffer->getMemBufferRef(), Kind, /*IsSystem*/ false);

  return true;
}

void ExtractAPIAction::EndSourceFileAction() {
  ImplEndSourceFileAction(getCompilerInstance());
}

std::unique_ptr<ASTConsumer>
WrappingExtractAPIAction::CreateASTConsumer(CompilerInstance &CI,
                                            StringRef InFile) {
  auto OtherConsumer = WrapperFrontendAction::CreateASTConsumer(CI, InFile);
  if (!OtherConsumer)
    return nullptr;

  CreatedASTConsumer = true;

  ProductName = CI.getFrontendOpts().ProductName;
  auto InputFilename = toolchain::sys::path::filename(InFile);
  OS = createAdditionalSymbolGraphFile(CI, InputFilename);

  // Now that we have enough information about the language options and the
  // target triple, let's create the APISet before anyone uses it.
  API = std::make_unique<APISet>(
      CI.getTarget().getTriple(),
      CI.getFrontendOpts().Inputs.back().getKind().getLanguage(), ProductName);

  CI.getPreprocessor().addPPCallbacks(std::make_unique<MacroCallback>(
      CI.getSourceManager(), *API, CI.getPreprocessor()));

  // Do not include location in anonymous decls.
  PrintingPolicy Policy = CI.getASTContext().getPrintingPolicy();
  Policy.AnonymousTagLocations = false;
  CI.getASTContext().setPrintingPolicy(Policy);

  if (!CI.getFrontendOpts().ExtractAPIIgnoresFileList.empty()) {
    toolchain::handleAllErrors(
        APIIgnoresList::create(CI.getFrontendOpts().ExtractAPIIgnoresFileList,
                               CI.getFileManager())
            .moveInto(IgnoresList),
        [&CI](const IgnoresFileNotFound &Err) {
          CI.getDiagnostics().Report(
              diag::err_extract_api_ignores_file_not_found)
              << Err.Path;
        });
  }

  auto WrappingConsumer =
      std::make_unique<WrappingExtractAPIConsumer>(CI.getASTContext(), *API);
  std::vector<std::unique_ptr<ASTConsumer>> Consumers;
  Consumers.push_back(std::move(OtherConsumer));
  Consumers.push_back(std::move(WrappingConsumer));

  return std::make_unique<MultiplexConsumer>(std::move(Consumers));
}

void WrappingExtractAPIAction::EndSourceFileAction() {
  // Invoke wrapped action's method.
  WrapperFrontendAction::EndSourceFileAction();

  if (CreatedASTConsumer) {
    ImplEndSourceFileAction(getCompilerInstance());
  }
}
