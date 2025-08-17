//===- USRGeneration.h - Routines for USR generation ------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_INDEX_USRGENERATION_H
#define LANGUAGE_CORE_INDEX_USRGENERATION_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
class ASTContext;
class Decl;
class LangOptions;
class MacroDefinitionRecord;
class Module;
class SourceLocation;
class SourceManager;
class QualType;

namespace index {

static inline StringRef getUSRSpacePrefix() {
  return "c:";
}

/// Generate a USR for a Decl, including the USR prefix.
/// \returns true if the results should be ignored, false otherwise.
bool generateUSRForDecl(const Decl *D, SmallVectorImpl<char> &Buf);
bool generateUSRForDecl(const Decl *D, SmallVectorImpl<char> &Buf,
                        const LangOptions &LangOpts);

/// Generate a USR fragment for an Objective-C class.
void generateUSRForObjCClass(StringRef Cls, raw_ostream &OS,
                             StringRef ExtSymbolDefinedIn = "",
                             StringRef CategoryContextExtSymbolDefinedIn = "");

/// Generate a USR fragment for an Objective-C class category.
void generateUSRForObjCCategory(StringRef Cls, StringRef Cat, raw_ostream &OS,
                                StringRef ClsExtSymbolDefinedIn = "",
                                StringRef CatExtSymbolDefinedIn = "");

/// Generate a USR fragment for an Objective-C instance variable.  The
/// complete USR can be created by concatenating the USR for the
/// encompassing class with this USR fragment.
void generateUSRForObjCIvar(StringRef Ivar, raw_ostream &OS);

/// Generate a USR fragment for an Objective-C method.
void generateUSRForObjCMethod(StringRef Sel, bool IsInstanceMethod,
                              raw_ostream &OS);

/// Generate a USR fragment for an Objective-C property.
void generateUSRForObjCProperty(StringRef Prop, bool isClassProp, raw_ostream &OS);

/// Generate a USR fragment for an Objective-C protocol.
void generateUSRForObjCProtocol(StringRef Prot, raw_ostream &OS,
                                StringRef ExtSymbolDefinedIn = "");

/// Generate USR fragment for a global (non-nested) enum.
void generateUSRForGlobalEnum(StringRef EnumName, raw_ostream &OS,
                              StringRef ExtSymbolDefinedIn = "");

/// Generate a USR fragment for an enum constant.
void generateUSRForEnumConstant(StringRef EnumConstantName, raw_ostream &OS);

/// Generate a USR for a macro, including the USR prefix.
///
/// \returns true on error, false on success.
bool generateUSRForMacro(const MacroDefinitionRecord *MD,
                         const SourceManager &SM, SmallVectorImpl<char> &Buf);
bool generateUSRForMacro(StringRef MacroName, SourceLocation Loc,
                         const SourceManager &SM, SmallVectorImpl<char> &Buf);

/// Generates a USR for a type.
///
/// \return true on error, false on success.
bool generateUSRForType(QualType T, ASTContext &Ctx,
                        SmallVectorImpl<char> &Buf);
bool generateUSRForType(QualType T, ASTContext &Ctx, SmallVectorImpl<char> &Buf,
                        const LangOptions &LangOpts);

/// Generate a USR for a module, including the USR prefix.
/// \returns true on error, false on success.
bool generateFullUSRForModule(const Module *Mod, raw_ostream &OS);

/// Generate a USR for a top-level module name, including the USR prefix.
/// \returns true on error, false on success.
bool generateFullUSRForTopLevelModuleName(StringRef ModName, raw_ostream &OS);

/// Generate a USR fragment for a module.
/// \returns true on error, false on success.
bool generateUSRFragmentForModule(const Module *Mod, raw_ostream &OS);

/// Generate a USR fragment for a module name.
/// \returns true on error, false on success.
bool generateUSRFragmentForModuleName(StringRef ModName, raw_ostream &OS);


} // namespace index
} // namespace language::Core

#endif // LANGUAGE_CORE_INDEX_USRGENERATION_H

