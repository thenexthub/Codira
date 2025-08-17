//===--- RawCommentList.h - Classes for processing raw comments -*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_AST_RAWCOMMENTLIST_H
#define LANGUAGE_CORE_AST_RAWCOMMENTLIST_H

#include "language/Core/Basic/CommentOptions.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/Allocator.h"
#include <map>

namespace language::Core {

class ASTContext;
class ASTReader;
class Decl;
class DiagnosticsEngine;
class Preprocessor;
class SourceManager;

namespace comments {
  class FullComment;
} // end namespace comments

class RawComment {
public:
  enum CommentKind {
    RCK_Invalid,      ///< Invalid comment
    RCK_OrdinaryBCPL, ///< Any normal BCPL comments
    RCK_OrdinaryC,    ///< Any normal C comment
    RCK_BCPLSlash,    ///< \code /// stuff \endcode
    RCK_BCPLExcl,     ///< \code //! stuff \endcode
    RCK_JavaDoc,      ///< \code /** stuff */ \endcode
    RCK_Qt,           ///< \code /*! stuff */ \endcode, also used by HeaderDoc
    RCK_Merged        ///< Two or more documentation comments merged together
  };

  RawComment() : Kind(RCK_Invalid), IsAlmostTrailingComment(false) { }

  RawComment(const SourceManager &SourceMgr, SourceRange SR,
             const CommentOptions &CommentOpts, bool Merged);

  CommentKind getKind() const LLVM_READONLY {
    return (CommentKind) Kind;
  }

  bool isInvalid() const LLVM_READONLY {
    return Kind == RCK_Invalid;
  }

  bool isMerged() const LLVM_READONLY {
    return Kind == RCK_Merged;
  }

  /// Is this comment attached to any declaration?
  bool isAttached() const LLVM_READONLY {
    return IsAttached;
  }

  void setAttached() {
    IsAttached = true;
  }

  /// Returns true if it is a comment that should be put after a member:
  /// \code ///< stuff \endcode
  /// \code //!< stuff \endcode
  /// \code /**< stuff */ \endcode
  /// \code /*!< stuff */ \endcode
  bool isTrailingComment() const LLVM_READONLY {
    return IsTrailingComment;
  }

  /// Returns true if it is a probable typo:
  /// \code //< stuff \endcode
  /// \code /*< stuff */ \endcode
  bool isAlmostTrailingComment() const LLVM_READONLY {
    return IsAlmostTrailingComment;
  }

  /// Returns true if this comment is not a documentation comment.
  bool isOrdinary() const LLVM_READONLY {
    return ((Kind == RCK_OrdinaryBCPL) || (Kind == RCK_OrdinaryC));
  }

  /// Returns true if this comment any kind of a documentation comment.
  bool isDocumentation() const LLVM_READONLY {
    return !isInvalid() && !isOrdinary();
  }

  /// Returns raw comment text with comment markers.
  StringRef getRawText(const SourceManager &SourceMgr) const {
    if (RawTextValid)
      return RawText;

    RawText = getRawTextSlow(SourceMgr);
    RawTextValid = true;
    return RawText;
  }

  SourceRange getSourceRange() const LLVM_READONLY { return Range; }
  SourceLocation getBeginLoc() const LLVM_READONLY { return Range.getBegin(); }
  SourceLocation getEndLoc() const LLVM_READONLY { return Range.getEnd(); }

  const char *getBriefText(const ASTContext &Context) const {
    if (BriefTextValid)
      return BriefText;

    return extractBriefText(Context);
  }

  bool hasUnsupportedSplice(const SourceManager &SourceMgr) const {
    if (!isInvalid())
      return false;
    StringRef Text = getRawText(SourceMgr);
    if (Text.size() < 6 || Text[0] != '/')
      return false;
    if (Text[1] == '*')
      return Text[Text.size() - 1] != '/' || Text[Text.size() - 2] != '*';
    return Text[1] != '/';
  }

  /// Returns sanitized comment text, suitable for presentation in editor UIs.
  /// E.g. will transform:
  ///     // This is a long multiline comment.
  ///     //   Parts of it  might be indented.
  ///     /* The comments styles might be mixed. */
  ///  into
  ///     "This is a long multiline comment.\n"
  ///     "  Parts of it  might be indented.\n"
  ///     "The comments styles might be mixed."
  /// Also removes leading indentation and sanitizes some common cases:
  ///     /* This is a first line.
  ///      *   This is a second line. It is indented.
  ///      * This is a third line. */
  /// and
  ///     /* This is a first line.
  ///          This is a second line. It is indented.
  ///     This is a third line. */
  /// will both turn into:
  ///     "This is a first line.\n"
  ///     "  This is a second line. It is indented.\n"
  ///     "This is a third line."
  std::string getFormattedText(const SourceManager &SourceMgr,
                               DiagnosticsEngine &Diags) const;

  struct CommentLine {
    std::string Text;
    PresumedLoc Begin;
    PresumedLoc End;

    CommentLine(StringRef Text, PresumedLoc Begin, PresumedLoc End)
        : Text(Text), Begin(Begin), End(End) {}
  };

  /// Returns sanitized comment text as separated lines with locations in
  /// source, suitable for further processing and rendering requiring source
  /// locations.
  std::vector<CommentLine> getFormattedLines(const SourceManager &SourceMgr,
                                             DiagnosticsEngine &Diags) const;

  /// Parse the comment, assuming it is attached to decl \c D.
  comments::FullComment *parse(const ASTContext &Context,
                               const Preprocessor *PP, const Decl *D) const;

private:
  SourceRange Range;

  mutable StringRef RawText;
  mutable const char *BriefText = nullptr;

  LLVM_PREFERRED_TYPE(bool)
  mutable unsigned RawTextValid : 1;
  LLVM_PREFERRED_TYPE(bool)
  mutable unsigned BriefTextValid : 1;

  LLVM_PREFERRED_TYPE(CommentKind)
  unsigned Kind : 3;

  /// True if comment is attached to a declaration in ASTContext.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IsAttached : 1;

  LLVM_PREFERRED_TYPE(bool)
  unsigned IsTrailingComment : 1;
  LLVM_PREFERRED_TYPE(bool)
  unsigned IsAlmostTrailingComment : 1;

  /// Constructor for AST deserialization.
  RawComment(SourceRange SR, CommentKind K, bool IsTrailingComment,
             bool IsAlmostTrailingComment) :
    Range(SR), RawTextValid(false), BriefTextValid(false), Kind(K),
    IsAttached(false), IsTrailingComment(IsTrailingComment),
    IsAlmostTrailingComment(IsAlmostTrailingComment)
  { }

  StringRef getRawTextSlow(const SourceManager &SourceMgr) const;

  const char *extractBriefText(const ASTContext &Context) const;

  friend class ASTReader;
};

/// This class represents all comments included in the translation unit,
/// sorted in order of appearance in the translation unit.
class RawCommentList {
public:
  RawCommentList(SourceManager &SourceMgr) : SourceMgr(SourceMgr) {}

  void addComment(const RawComment &RC, const CommentOptions &CommentOpts,
                  toolchain::BumpPtrAllocator &Allocator);

  /// \returns A mapping from an offset of the start of the comment to the
  /// comment itself, or nullptr in case there are no comments in \p File.
  const std::map<unsigned, RawComment *> *getCommentsInFile(FileID File) const;

  bool empty() const;

  unsigned getCommentBeginLine(RawComment *C, FileID File,
                               unsigned Offset) const;
  unsigned getCommentEndOffset(RawComment *C) const;

private:
  SourceManager &SourceMgr;
  // mapping: FileId -> comment begin offset -> comment
  toolchain::DenseMap<FileID, std::map<unsigned, RawComment *>> OrderedComments;
  mutable toolchain::DenseMap<RawComment *, unsigned> CommentBeginLine;
  mutable toolchain::DenseMap<RawComment *, unsigned> CommentEndOffset;

  friend class ASTReader;
  friend class ASTWriter;
};

} // end namespace language::Core

#endif
