//===--- MacroPPCallbacks.h -------------------------------------*- C++ -*-===//
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
//  This file defines implementation for the macro preprocessors callbacks.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_MACROPPCALLBACKS_H
#define LANGUAGE_CORE_LIB_CODEGEN_MACROPPCALLBACKS_H

#include "language/Core/Lex/PPCallbacks.h"

namespace toolchain {
class DIMacroFile;
}
namespace language::Core {
class Preprocessor;
class MacroInfo;
class CodeGenerator;

class MacroPPCallbacks : public PPCallbacks {
  /// A pointer to code generator, where debug info generator can be found.
  CodeGenerator *Gen;

  /// Preprocessor.
  Preprocessor &PP;

  /// Location of recent included file, used for line number.
  SourceLocation LastHashLoc;

  /// Counts current number of command line included files, which were entered
  /// and were not exited yet.
  int EnteredCommandLineIncludeFiles = 0;

  enum FileScopeStatus {
    NoScope = 0,              // Scope is not initialized yet.
    InitializedScope,         // Main file scope is initialized but not set yet.
    BuiltinScope,             // <built-in> and <command line> file scopes.
    CommandLineIncludeScope,  // Included file, from <command line> file, scope.
    MainFileScope             // Main file scope.
  };
  FileScopeStatus Status;

  /// Parent contains all entered files that were not exited yet according to
  /// the inclusion order.
  toolchain::SmallVector<toolchain::DIMacroFile *, 4> Scopes;

  /// Get current DIMacroFile scope.
  /// \return current DIMacroFile scope or nullptr if there is no such scope.
  toolchain::DIMacroFile *getCurrentScope();

  /// Get current line location or invalid location.
  /// \param Loc current line location.
  /// \return current line location \p `Loc`, or invalid location if it's in a
  ///         skipped file scope.
  SourceLocation getCorrectLocation(SourceLocation Loc);

  /// Use the passed preprocessor to write the macro name and value from the
  /// given macro info and identifier info into the given \p `Name` and \p
  /// `Value` output streams.
  ///
  /// \param II Identifier info, used to get the Macro name.
  /// \param MI Macro info, used to get the Macro argumets and values.
  /// \param PP Preprocessor.
  /// \param [out] Name Place holder for returned macro name and arguments.
  /// \param [out] Value Place holder for returned macro value.
  static void writeMacroDefinition(const IdentifierInfo &II,
                                   const MacroInfo &MI, Preprocessor &PP,
                                   raw_ostream &Name, raw_ostream &Value);

  /// Update current file scope status to next file scope.
  void updateStatusToNextScope();

  /// Handle the case when entering a file.
  ///
  /// \param Loc Indicates the new location.
  void FileEntered(SourceLocation Loc);

  /// Handle the case when exiting a file.
  ///
  /// \param Loc Indicates the new location.
  void FileExited(SourceLocation Loc);

public:
  MacroPPCallbacks(CodeGenerator *Gen, Preprocessor &PP);

  /// Callback invoked whenever a source file is entered or exited.
  ///
  /// \param Loc Indicates the new location.
  /// \param PrevFID the file that was exited if \p Reason is ExitFile.
  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID = FileID()) override;

  /// Callback invoked whenever a directive (#xxx) is processed.
  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *SuggestedModule,
                          bool ModuleImported,
                          SrcMgr::CharacteristicKind FileType) override;

  /// Hook called whenever a macro definition is seen.
  void MacroDefined(const Token &MacroNameTok,
                    const MacroDirective *MD) override;

  /// Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  void MacroUndefined(const Token &MacroNameTok, const MacroDefinition &MD,
                      const MacroDirective *Undef) override;
};

} // end namespace language::Core

#endif
