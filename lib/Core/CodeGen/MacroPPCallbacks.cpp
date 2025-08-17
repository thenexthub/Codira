//===--- MacroPPCallbacks.cpp ---------------------------------------------===//
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
//  This file contains implementation for the macro preprocessors callbacks.
//
//===----------------------------------------------------------------------===//

#include "MacroPPCallbacks.h"
#include "CGDebugInfo.h"
#include "language/Core/CodeGen/ModuleBuilder.h"
#include "language/Core/Lex/MacroInfo.h"
#include "language/Core/Lex/Preprocessor.h"

using namespace language::Core;

void MacroPPCallbacks::writeMacroDefinition(const IdentifierInfo &II,
                                            const MacroInfo &MI,
                                            Preprocessor &PP, raw_ostream &Name,
                                            raw_ostream &Value) {
  Name << II.getName();

  if (MI.isFunctionLike()) {
    Name << '(';
    if (!MI.param_empty()) {
      MacroInfo::param_iterator AI = MI.param_begin(), E = MI.param_end();
      for (; AI + 1 != E; ++AI) {
        Name << (*AI)->getName();
        Name << ',';
      }

      // Last argument.
      if ((*AI)->getName() == "__VA_ARGS__")
        Name << "...";
      else
        Name << (*AI)->getName();
    }

    if (MI.isGNUVarargs())
      // #define foo(x...)
      Name << "...";

    Name << ')';
  }

  SmallString<128> SpellingBuffer;
  bool First = true;
  for (const auto &T : MI.tokens()) {
    if (!First && T.hasLeadingSpace())
      Value << ' ';

    Value << PP.getSpelling(T, SpellingBuffer);
    First = false;
  }
}

MacroPPCallbacks::MacroPPCallbacks(CodeGenerator *Gen, Preprocessor &PP)
    : Gen(Gen), PP(PP), Status(NoScope) {}

// This is the expected flow of enter/exit compiler and user files:
// - Main File Enter
//   - <built-in> file enter
//     {Compiler macro definitions} - (Line=0, no scope)
//     - (Optional) <command line> file enter
//     {Command line macro definitions} - (Line=0, no scope)
//     - (Optional) <command line> file exit
//     {Command line file includes} - (Line=0, Main file scope)
//       {macro definitions and file includes} - (Line!=0, Parent scope)
//   - <built-in> file exit
//   {User code macro definitions and file includes} - (Line!=0, Parent scope)

toolchain::DIMacroFile *MacroPPCallbacks::getCurrentScope() {
  if (Status == MainFileScope || Status == CommandLineIncludeScope)
    return Scopes.back();
  return nullptr;
}

SourceLocation MacroPPCallbacks::getCorrectLocation(SourceLocation Loc) {
  if (Status == MainFileScope || EnteredCommandLineIncludeFiles)
    return Loc;

  // While parsing skipped files, location of macros is invalid.
  // Invalid location represents line zero.
  return SourceLocation();
}

void MacroPPCallbacks::updateStatusToNextScope() {
  switch (Status) {
  case NoScope:
    Status = InitializedScope;
    break;
  case InitializedScope:
    Status = BuiltinScope;
    break;
  case BuiltinScope:
    Status = CommandLineIncludeScope;
    break;
  case CommandLineIncludeScope:
    Status = MainFileScope;
    break;
  case MainFileScope:
    toolchain_unreachable("There is no next scope, already in the final scope");
  }
}

void MacroPPCallbacks::FileEntered(SourceLocation Loc) {
  SourceLocation LineLoc = getCorrectLocation(LastHashLoc);
  switch (Status) {
  case NoScope:
    updateStatusToNextScope();
    break;
  case InitializedScope:
    updateStatusToNextScope();
    return;
  case BuiltinScope:
    if (PP.getSourceManager().isWrittenInCommandLineFile(Loc))
      return;
    updateStatusToNextScope();
    [[fallthrough]];
  case CommandLineIncludeScope:
    EnteredCommandLineIncludeFiles++;
    break;
  case MainFileScope:
    break;
  }

  Scopes.push_back(Gen->getCGDebugInfo()->CreateTempMacroFile(getCurrentScope(),
                                                              LineLoc, Loc));
}

void MacroPPCallbacks::FileExited(SourceLocation Loc) {
  switch (Status) {
  default:
    toolchain_unreachable("Do not expect to exit a file from current scope");
  case BuiltinScope:
    if (!PP.getSourceManager().isWrittenInBuiltinFile(Loc))
      // Skip next scope and change status to MainFileScope.
      Status = MainFileScope;
    return;
  case CommandLineIncludeScope:
    if (!EnteredCommandLineIncludeFiles) {
      updateStatusToNextScope();
      return;
    }
    EnteredCommandLineIncludeFiles--;
    break;
  case MainFileScope:
    break;
  }

  Scopes.pop_back();
}

void MacroPPCallbacks::FileChanged(SourceLocation Loc, FileChangeReason Reason,
                                   SrcMgr::CharacteristicKind FileType,
                                   FileID PrevFID) {
  // Only care about enter file or exit file changes.
  if (Reason == EnterFile)
    FileEntered(Loc);
  else if (Reason == ExitFile)
    FileExited(Loc);
}

void MacroPPCallbacks::InclusionDirective(
    SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName,
    bool IsAngled, CharSourceRange FilenameRange, OptionalFileEntryRef File,
    StringRef SearchPath, StringRef RelativePath, const Module *SuggestedModule,
    bool ModuleImported, SrcMgr::CharacteristicKind FileType) {

  // Record the line location of the current included file.
  LastHashLoc = HashLoc;
}

void MacroPPCallbacks::MacroDefined(const Token &MacroNameTok,
                                    const MacroDirective *MD) {
  IdentifierInfo *Id = MacroNameTok.getIdentifierInfo();
  SourceLocation location = getCorrectLocation(MacroNameTok.getLocation());
  std::string NameBuffer, ValueBuffer;
  toolchain::raw_string_ostream Name(NameBuffer);
  toolchain::raw_string_ostream Value(ValueBuffer);
  writeMacroDefinition(*Id, *MD->getMacroInfo(), PP, Name, Value);
  Gen->getCGDebugInfo()->CreateMacro(getCurrentScope(),
                                     toolchain::dwarf::DW_MACINFO_define, location,
                                     NameBuffer, ValueBuffer);
}

void MacroPPCallbacks::MacroUndefined(const Token &MacroNameTok,
                                      const MacroDefinition &MD,
                                      const MacroDirective *Undef) {
  IdentifierInfo *Id = MacroNameTok.getIdentifierInfo();
  SourceLocation location = getCorrectLocation(MacroNameTok.getLocation());
  Gen->getCGDebugInfo()->CreateMacro(getCurrentScope(),
                                     toolchain::dwarf::DW_MACINFO_undef, location,
                                     Id->getName(), "");
}
