//===--- CommentSema.h - Doxygen comment semantic analysis ------*- C++ -*-===//
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
//  This file defines the semantic analysis class for Doxygen comments.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_COMMENTSEMA_H
#define LANGUAGE_CORE_AST_COMMENTSEMA_H

#include "language/Core/AST/Comment.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"

namespace language::Core {
class Decl;
class SourceMgr;
class Preprocessor;

namespace comments {
class CommandTraits;

class Sema {
  Sema(const Sema &) = delete;
  void operator=(const Sema &) = delete;

  /// Allocator for AST nodes.
  toolchain::BumpPtrAllocator &Allocator;

  /// Source manager for the comment being parsed.
  const SourceManager &SourceMgr;

  DiagnosticsEngine &Diags;

  CommandTraits &Traits;

  const Preprocessor *PP;

  /// Information about the declaration this comment is attached to.
  DeclInfo *ThisDeclInfo;

  /// Comment AST nodes that correspond to parameter names in
  /// \c TemplateParameters.
  ///
  /// Contains a valid value if \c DeclInfo->IsFilled is true.
  toolchain::StringMap<TParamCommandComment *> TemplateParameterDocs;

  /// AST node for the \command and its aliases.
  const BlockCommandComment *BriefCommand;

  /// AST node for the \\headerfile command.
  const BlockCommandComment *HeaderfileCommand;

  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) {
    return Diags.Report(Loc, DiagID);
  }

  /// A stack of HTML tags that are currently open (not matched with closing
  /// tags).
  SmallVector<HTMLStartTagComment *, 8> HTMLOpenTags;

public:
  Sema(toolchain::BumpPtrAllocator &Allocator, const SourceManager &SourceMgr,
       DiagnosticsEngine &Diags, CommandTraits &Traits,
       const Preprocessor *PP);

  void setDecl(const Decl *D);

  /// Returns a copy of array, owned by Sema's allocator.
  template<typename T>
  ArrayRef<T> copyArray(ArrayRef<T> Source) {
    if (!Source.empty())
      return Source.copy(Allocator);
    return {};
  }

  ParagraphComment *actOnParagraphComment(
      ArrayRef<InlineContentComment *> Content);

  BlockCommandComment *actOnBlockCommandStart(SourceLocation LocBegin,
                                              SourceLocation LocEnd,
                                              unsigned CommandID,
                                              CommandMarkerKind CommandMarker);

  void actOnBlockCommandArgs(BlockCommandComment *Command,
                             ArrayRef<BlockCommandComment::Argument> Args);

  void actOnBlockCommandFinish(BlockCommandComment *Command,
                               ParagraphComment *Paragraph);

  ParamCommandComment *actOnParamCommandStart(SourceLocation LocBegin,
                                              SourceLocation LocEnd,
                                              unsigned CommandID,
                                              CommandMarkerKind CommandMarker);

  void actOnParamCommandDirectionArg(ParamCommandComment *Command,
                                     SourceLocation ArgLocBegin,
                                     SourceLocation ArgLocEnd,
                                     StringRef Arg);

  void actOnParamCommandParamNameArg(ParamCommandComment *Command,
                                     SourceLocation ArgLocBegin,
                                     SourceLocation ArgLocEnd,
                                     StringRef Arg);

  void actOnParamCommandFinish(ParamCommandComment *Command,
                               ParagraphComment *Paragraph);

  TParamCommandComment *actOnTParamCommandStart(SourceLocation LocBegin,
                                                SourceLocation LocEnd,
                                                unsigned CommandID,
                                                CommandMarkerKind CommandMarker);

  void actOnTParamCommandParamNameArg(TParamCommandComment *Command,
                                      SourceLocation ArgLocBegin,
                                      SourceLocation ArgLocEnd,
                                      StringRef Arg);

  void actOnTParamCommandFinish(TParamCommandComment *Command,
                                ParagraphComment *Paragraph);

  InlineCommandComment *actOnInlineCommand(SourceLocation CommandLocBegin,
                                           SourceLocation CommandLocEnd,
                                           unsigned CommandID,
                                           CommandMarkerKind CommandMarker,
                                           ArrayRef<Comment::Argument> Args);

  InlineContentComment *actOnUnknownCommand(SourceLocation LocBegin,
                                            SourceLocation LocEnd,
                                            StringRef CommandName);

  InlineContentComment *actOnUnknownCommand(SourceLocation LocBegin,
                                            SourceLocation LocEnd,
                                            unsigned CommandID);

  TextComment *actOnText(SourceLocation LocBegin,
                         SourceLocation LocEnd,
                         StringRef Text);

  VerbatimBlockComment *actOnVerbatimBlockStart(SourceLocation Loc,
                                                unsigned CommandID);

  VerbatimBlockLineComment *actOnVerbatimBlockLine(SourceLocation Loc,
                                                   StringRef Text);

  void actOnVerbatimBlockFinish(VerbatimBlockComment *Block,
                                SourceLocation CloseNameLocBegin,
                                StringRef CloseName,
                                ArrayRef<VerbatimBlockLineComment *> Lines);

  VerbatimLineComment *actOnVerbatimLine(SourceLocation LocBegin,
                                         unsigned CommandID,
                                         SourceLocation TextBegin,
                                         StringRef Text);

  HTMLStartTagComment *actOnHTMLStartTagStart(SourceLocation LocBegin,
                                              StringRef TagName);

  void actOnHTMLStartTagFinish(HTMLStartTagComment *Tag,
                               ArrayRef<HTMLStartTagComment::Attribute> Attrs,
                               SourceLocation GreaterLoc,
                               bool IsSelfClosing);

  HTMLEndTagComment *actOnHTMLEndTag(SourceLocation LocBegin,
                                     SourceLocation LocEnd,
                                     StringRef TagName);

  FullComment *actOnFullComment(ArrayRef<BlockContentComment *> Blocks);

private:
  void checkBlockCommandEmptyParagraph(BlockCommandComment *Command);

  void checkReturnsCommand(const BlockCommandComment *Command);

  /// Emit diagnostics about duplicate block commands that should be
  /// used only once per comment, e.g., \and \\returns.
  void checkBlockCommandDuplicate(const BlockCommandComment *Command);

  void checkDeprecatedCommand(const BlockCommandComment *Comment);

  void checkFunctionDeclVerbatimLine(const BlockCommandComment *Comment);

  void checkContainerDeclVerbatimLine(const BlockCommandComment *Comment);

  void checkContainerDecl(const BlockCommandComment *Comment);

  /// Resolve parameter names to parameter indexes in function declaration.
  /// Emit diagnostics about unknown parameters.
  void resolveParamCommandIndexes(const FullComment *FC);

  /// \returns \c true if the declaration that this comment is attached to
  /// is a pointer to function/method/block type or has such a type.
  bool involvesFunctionType();

  bool isFunctionDecl();
  bool isAnyFunctionDecl();

  /// \returns \c true if declaration that this comment is attached to declares
  /// a function pointer.
  bool isFunctionPointerVarDecl();
  bool isFunctionOrMethodVariadic();
  bool isObjCMethodDecl();
  bool isObjCPropertyDecl();
  bool isTemplateOrSpecialization();
  bool isRecordLikeDecl();
  bool isClassOrStructDecl();
  /// \return \c true if the declaration that this comment is attached to
  /// declares either struct, class or tag typedef.
  bool isClassOrStructOrTagTypedefDecl();
  bool isUnionDecl();
  bool isObjCInterfaceDecl();
  bool isObjCProtocolDecl();
  bool isClassTemplateDecl();
  bool isFunctionTemplateDecl();

  ArrayRef<const ParmVarDecl *> getParamVars();

  /// Extract all important semantic information from
  /// \c ThisDeclInfo->ThisDecl into \c ThisDeclInfo members.
  void inspectThisDecl();

  /// Returns index of a function parameter with a given name.
  unsigned resolveParmVarReference(StringRef Name,
                                   ArrayRef<const ParmVarDecl *> ParamVars);

  /// Returns index of a function parameter with the name closest to a given
  /// typo.
  unsigned correctTypoInParmVarReference(StringRef Typo,
                                         ArrayRef<const ParmVarDecl *> ParamVars);

  bool resolveTParamReference(StringRef Name,
                              const TemplateParameterList *TemplateParameters,
                              SmallVectorImpl<unsigned> *Position);

  StringRef correctTypoInTParamReference(
                              StringRef Typo,
                              const TemplateParameterList *TemplateParameters);

  InlineCommandRenderKind getInlineCommandRenderKind(StringRef Name) const;
};

} // end namespace comments
} // end namespace language::Core

#endif

