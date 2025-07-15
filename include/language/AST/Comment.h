//===--- Comment.h - Codira-specific comment parsing -------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_COMMENT_H
#define LANGUAGE_AST_COMMENT_H

#include "language/Markup/Markup.h"
#include <optional>

namespace language {
class Decl;
class TypeDecl;
struct RawComment;

class DocComment {
  const Decl *D;
  language::markup::Document *Doc = nullptr;
  language::markup::CommentParts Parts;

  DocComment(const Decl *D, language::markup::Document *Doc,
             language::markup::CommentParts Parts)
      : D(D), Doc(Doc), Parts(Parts) {}

public:
  static DocComment *create(const Decl *D, language::markup::MarkupContext &MC,
                            RawComment RC);

  void addInheritanceNote(language::markup::MarkupContext &MC, TypeDecl *base);

  const Decl *getDecl() const { return D; }
  void setDecl(const Decl *D) { this->D = D; }

  const language::markup::Document *getDocument() const { return Doc; }

  language::markup::CommentParts getParts() const {
    return Parts;
  }

  ArrayRef<StringRef> getTags() const {
    return toolchain::ArrayRef(Parts.Tags.begin(), Parts.Tags.end());
  }

  std::optional<const language::markup::Paragraph *> getBrief() const {
    return Parts.Brief;
  }

  std::optional<const language::markup::ReturnsField *> getReturnsField() const {
    return Parts.ReturnsField;
  }

  std::optional<const language::markup::ThrowsField *> getThrowsField() const {
    return Parts.ThrowsField;
  }

  ArrayRef<const language::markup::ParamField *> getParamFields() const {
    return Parts.ParamFields;
  }

  ArrayRef<const language::markup::MarkupASTNode *> getBodyNodes() const {
    return Parts.BodyNodes;
  }

  std::optional<const markup::LocalizationKeyField *>
  getLocalizationKeyField() const {
    return Parts.LocalizationKeyField;
  }

  bool isEmpty() const {
    return Parts.isEmpty();
  }

  // Only allow allocation using the allocator in MarkupContext or by
  // placement new.
  void *operator new(size_t Bytes, language::markup::MarkupContext &MC,
                     unsigned Alignment = alignof(DocComment));
  void *operator new(size_t Bytes, void *Mem) {
    assert(Mem);
    return Mem;
  }

  // Make vanilla new/delete illegal.
  void *operator new(size_t Bytes) = delete;
  void operator delete(void *Data) = delete;
};

/// Get a parsed documentation comment for the declaration, if there is one.
DocComment *getSingleDocComment(language::markup::MarkupContext &Context,
                                const Decl *D);

/// Extract comments parts from the given Markup node.
language::markup::CommentParts
extractCommentParts(language::markup::MarkupContext &MC,
                    language::markup::MarkupASTNode *Node);

/// Extract brief comment from \p RC, and print it to \p OS .
void printBriefComment(RawComment RC, toolchain::raw_ostream &OS);

/// Describes the intended serialization target for a doc comment.
enum class DocCommentSerializationTarget : uint8_t {
  /// The doc comment should not be serialized.
  None = 0,

  /// The doc comment should only be serialized in the 'languagesourceinfo'.
  SourceInfoOnly,

  /// The doc comment should be serialized in both the 'languagedoc' and
  /// 'languagesourceinfo'.
  CodiraDocAndSourceInfo,
};

/// Retrieve the expected serialization target for a documentation comment
/// attached to the given decl.
DocCommentSerializationTarget
getDocCommentSerializationTargetFor(const Decl *D);

} // namespace language

#endif // TOOLCHAIN_LANGUAGE_AST_COMMENT_H
