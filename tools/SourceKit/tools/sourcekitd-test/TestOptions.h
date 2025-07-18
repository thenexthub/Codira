//===--- TestOptions.h - ----------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKITD_TEST_TESTOPTIONS_H
#define TOOLCHAIN_SOURCEKITD_TEST_TESTOPTIONS_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringMap.h"
#include <optional>
#include <string>

namespace sourcekitd_test {

enum class SourceKitRequest {
  None,
  ProtocolVersion,
  CompilerVersion,
  DemangleNames,
  MangleSimpleClasses,
  Index,
  CodeComplete,
  CodeCompleteOpen,
  CodeCompleteClose,
  CodeCompleteUpdate,
  CodeCompleteCacheOnDisk,
  CodeCompleteSetPopularAPI,
  TypeContextInfo,
  ConformingMethodList,
  ActiveRegions,
  CursorInfo,
  RangeInfo,
  RelatedIdents,
  SyntaxMap,
  Structure,
  Format,
  ExpandPlaceholder,
  DocInfo,
  SemanticInfo,
  InterfaceGen,
  InterfaceGenOpen,
  FindUSR,
  FindInterfaceDoc,
  Open,
  Close,
  Edit,
  PrintAnnotations,
  PrintDiags,
  ExtractComment,
  ModuleGroups,
  FindRenameRanges,
  FindLocalRenameRanges,
  NameTranslation,
  MarkupToXML,
  Statistics,
  EnableCompileNotifications,
  CollectExpressionType,
  CollectVariableType,
  GlobalConfiguration,
  DependencyUpdated,
  Diagnostics,
  SemanticTokens,
  Compile,
  CompileClose,
  SyntacticMacroExpansion,
  IndexToStore,
#define SEMANTIC_REFACTORING(KIND, NAME, ID) KIND,
#include "language/Refactoring/RefactoringKinds.def"
};

struct TestOptions {
  SourceKitRequest Request = SourceKitRequest::None;
  std::vector<std::string> Inputs;
  /// The name of the underlying \c SourceFile.
  std::string SourceFile;
  /// The path to the primary file when building the AST, ie. when making a
  /// request from inside a macro expansion, this would be the real file on
  /// disk where as \c SourceFile is the name of the expansion's buffer.
  /// Defaults to \c SourceFile when not given.
  std::string PrimaryFile;
  std::string TextInputFile;
  std::string JsonRequestPath;
  std::string RenameSpecPath;
  std::optional<std::string> SourceText;
  std::string ModuleGroupName;
  std::string InterestedUSR;
  unsigned Line = 0;
  unsigned Col = 0;
  unsigned EndLine = 0;
  unsigned EndCol = 0;
  std::optional<unsigned> Offset;
  unsigned Length = 0;
  std::string CodiraVersion;
  bool PassVersionAsString = false;
  std::optional<std::string> ReplaceText;
  std::string ModuleName;
  std::string HeaderPath;
  bool PassAsSourceText = false;
  std::string CachePath;
  toolchain::SmallVector<std::string, 4> RequestOptions;
  toolchain::ArrayRef<const char *> CompilerArgs;
  std::string ModuleCachePath;
  bool UsingCodiraArgs;
  std::string USR;
  std::string CodiraName;
  std::string ObjCName;
  std::string ObjCSelector;
  std::string Name;
  /// An ID that can be used to cancel this request.
  std::string RequestId;
  /// If not empty, all requests with this ID should be cancelled.
  std::string CancelRequest;
  /// If set, simulate that the request takes x ms longer than it actually
  /// does. The request can be cancelled while waiting this duration.
  std::optional<uint64_t> SimulateLongRequest;
  bool CheckInterfaceIsASCII = false;
  bool UsedSema = false;
  bool PrintRequest = true;
  bool PrintResponseAsJSON = false;
  bool PrintRawResponse = false;
  bool PrintResponse = true;
  bool SimplifiedDemangling = false;
  bool SynthesizedExtensions = false;
  bool CollectActionables = false;
  bool isAsyncRequest = false;
  bool timeRequest = false;
  bool measureInstructions = false;
  bool DisableImplicitConcurrencyModuleImport = false;
  bool DisableImplicitStringProcessingModuleImport = false;
  std::optional<unsigned> CompletionCheckDependencyInterval;
  unsigned repeatRequest = 1;
  struct VFSFile {
    std::string path;
    bool passAsSourceText;
    VFSFile(std::string path, bool passAsSourceText)
        : path(std::move(path)), passAsSourceText(passAsSourceText) {}
  };
  toolchain::StringMap<VFSFile> VFSFiles;
  std::optional<std::string> VFSName;
  std::optional<bool> CancelOnSubsequentRequest;
  bool ShellExecution = false;
  std::string IndexStorePath;
  std::string IndexUnitOutputPath;
  bool parseArgs(toolchain::ArrayRef<const char *> Args);
  void printHelp(bool ShowHidden) const;
};

}

#endif
