//===--- DependencyGraph.cpp - Generate dependency file -------------------===//
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
//
// This code generates a header dependency graph in DOT format, for use
// with, e.g., GraphViz.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Frontend/Utils.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Frontend/FrontendDiagnostic.h"
#include "language/Core/Lex/PPCallbacks.h"
#include "language/Core/Lex/Preprocessor.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/Support/GraphWriter.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
namespace DOT = toolchain::DOT;

namespace {
class DependencyGraphCallback : public PPCallbacks {
  const Preprocessor *PP;
  std::string OutputFile;
  std::string SysRoot;
  toolchain::SetVector<FileEntryRef> AllFiles;
  using DependencyMap =
      toolchain::DenseMap<FileEntryRef, SmallVector<FileEntryRef, 2>>;

  DependencyMap Dependencies;

private:
  raw_ostream &writeNodeReference(raw_ostream &OS,
                                  const FileEntry *Node);
  void OutputGraphFile();

public:
  DependencyGraphCallback(const Preprocessor *_PP, StringRef OutputFile,
                          StringRef SysRoot)
      : PP(_PP), OutputFile(OutputFile.str()), SysRoot(SysRoot.str()) {}

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override;

  void EmbedDirective(SourceLocation HashLoc, StringRef FileName, bool IsAngled,
                      OptionalFileEntryRef File,
                      const LexEmbedParametersResult &Params) override;

  void EndOfMainFile() override {
    OutputGraphFile();
  }

};
}

void language::Core::AttachDependencyGraphGen(Preprocessor &PP, StringRef OutputFile,
                                     StringRef SysRoot) {
  PP.addPPCallbacks(std::make_unique<DependencyGraphCallback>(&PP, OutputFile,
                                                               SysRoot));
}

void DependencyGraphCallback::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, OptionalFileEntryRef File,
    StringRef SearchPath, StringRef RelativePath, const Module *SuggestedModule,
    bool ModuleImported, SrcMgr::CharacteristicKind FileType) {
  if (!File)
    return;

  SourceManager &SM = PP->getSourceManager();
  OptionalFileEntryRef FromFile =
      SM.getFileEntryRefForID(SM.getFileID(SM.getExpansionLoc(HashLoc)));
  if (!FromFile)
    return;

  Dependencies[*FromFile].push_back(*File);

  AllFiles.insert(*File);
  AllFiles.insert(*FromFile);
}

void DependencyGraphCallback::EmbedDirective(SourceLocation HashLoc, StringRef,
                                             bool, OptionalFileEntryRef File,
                                             const LexEmbedParametersResult &) {
  if (!File)
    return;

  SourceManager &SM = PP->getSourceManager();
  OptionalFileEntryRef FromFile =
      SM.getFileEntryRefForID(SM.getFileID(SM.getExpansionLoc(HashLoc)));
  if (!FromFile)
    return;

  Dependencies[*FromFile].push_back(*File);

  AllFiles.insert(*File);
  AllFiles.insert(*FromFile);
}

raw_ostream &
DependencyGraphCallback::writeNodeReference(raw_ostream &OS,
                                            const FileEntry *Node) {
  OS << "header_" << Node->getUID();
  return OS;
}

void DependencyGraphCallback::OutputGraphFile() {
  std::error_code EC;
  toolchain::raw_fd_ostream OS(OutputFile, EC, toolchain::sys::fs::OF_TextWithCRLF);
  if (EC) {
    PP->getDiagnostics().Report(diag::err_fe_error_opening) << OutputFile
                                                            << EC.message();
    return;
  }

  OS << "digraph \"dependencies\" {\n";

  // Write the nodes
  for (unsigned I = 0, N = AllFiles.size(); I != N; ++I) {
    // Write the node itself.
    OS.indent(2);
    writeNodeReference(OS, AllFiles[I]);
    OS << " [ shape=\"box\", label=\"";
    StringRef FileName = AllFiles[I].getName();
    FileName.consume_front(SysRoot);

    OS << DOT::EscapeString(std::string(FileName)) << "\"];\n";
  }

  // Write the edges
  for (DependencyMap::iterator F = Dependencies.begin(),
                            FEnd = Dependencies.end();
       F != FEnd; ++F) {
    for (unsigned I = 0, N = F->second.size(); I != N; ++I) {
      OS.indent(2);
      writeNodeReference(OS, F->first);
      OS << " -> ";
      writeNodeReference(OS, F->second[I]);
      OS << ";\n";
    }
  }
  OS << "}\n";
}

