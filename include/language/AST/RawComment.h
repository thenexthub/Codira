//===--- RawComment.h - Extraction of raw comments --------------*- C++ -*-===//
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

#ifndef SWIFT_AST_RAW_COMMENT_H
#define SWIFT_AST_RAW_COMMENT_H

#include "language/Basic/SourceLoc.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace language {

class SourceFile;
class SourceManager;

struct SingleRawComment {
  enum class CommentKind {
    OrdinaryLine,  ///< Any normal // comments
    OrdinaryBlock, ///< Any normal /* */ comment
    LineDoc,       ///< \code /// stuff \endcode
    BlockDoc,      ///< \code /** stuff */ \endcode
  };

  CharSourceRange Range;
  StringRef RawText;

  unsigned Kind : 8;
  unsigned ColumnIndent : 16;

  SingleRawComment(CharSourceRange Range, const SourceManager &SourceMgr);
  SingleRawComment(StringRef RawText, unsigned ColumnIndent);

  SingleRawComment(const SingleRawComment &) = default;
  SingleRawComment &operator=(const SingleRawComment &) = default;

  CommentKind getKind() const LLVM_READONLY {
    return static_cast<CommentKind>(Kind);
  }

  bool isOrdinary() const LLVM_READONLY {
    return getKind() == CommentKind::OrdinaryLine ||
           getKind() == CommentKind::OrdinaryBlock;
  }

  bool isLine() const LLVM_READONLY {
    return getKind() == CommentKind::OrdinaryLine ||
           getKind() == CommentKind::LineDoc;
  }

  bool isGyb() const {
    return RawText.starts_with("// ###");
  }
};

struct RawComment {
  ArrayRef<SingleRawComment> Comments;

  RawComment() {}
  RawComment(ArrayRef<SingleRawComment> Comments) : Comments(Comments) {}

  RawComment(const RawComment &) = default;
  RawComment &operator=(const RawComment &) = default;

  bool isEmpty() const {
    return Comments.empty();
  }

  CharSourceRange getCharSourceRange();
};

struct CommentInfo {
  StringRef Brief;
  RawComment Raw;
  uint32_t Group;
  uint32_t SourceOrder;
};

} // namespace language

#endif // LLVM_SWIFT_AST_RAW_COMMENT_H

