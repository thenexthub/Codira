//===--- DocComment.cpp - Extraction of doc comments ----------------------===//
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
///
/// \file
/// This file implements extraction of documentation comments from a Codira
/// Markup AST tree.
///
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/Comment.h"
#include "language/AST/Decl.h"
#include "language/AST/FileUnit.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/Types.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/RawComment.h"
#include "language/Basic/Assertions.h"
#include "language/Markup/Markup.h"
#include <queue>

using namespace language;

void *DocComment::operator new(size_t Bytes, language::markup::MarkupContext &MC,
                               unsigned Alignment) {
  return MC.allocate(Bytes, Alignment);
}

namespace {
std::optional<language::markup::ParamField *>
extractParamOutlineItem(language::markup::MarkupContext &MC,
                        language::markup::MarkupASTNode *Node) {

  auto Item = dyn_cast<language::markup::Item>(Node);
  if (!Item)
    return std::nullopt;

  auto Children = Item->getChildren();
  if (Children.empty())
    return std::nullopt;

  auto FirstChild = Children.front();
  auto FirstParagraph = dyn_cast<language::markup::Paragraph>(FirstChild);
  if (!FirstParagraph)
    return std::nullopt;

  auto FirstParagraphChildren = FirstParagraph->getChildren();
  if (FirstParagraphChildren.empty())
    return std::nullopt;

  auto ParagraphText =
      dyn_cast<language::markup::Text>(FirstParagraphChildren.front());
  if (!ParagraphText)
    return std::nullopt;

  StringRef Name;
  StringRef Remainder;
  std::tie(Name, Remainder) = ParagraphText->getLiteralContent().split(':');
  Name = Name.rtrim();

  if (Name.empty())
    return std::nullopt;

  ParagraphText->setLiteralContent(Remainder.ltrim());

  return language::markup::ParamField::create(MC, Name, Children);
}

bool extractParameterOutline(
    language::markup::MarkupContext &MC, language::markup::List *L,
    SmallVectorImpl<language::markup::ParamField *> &ParamFields) {
  SmallVector<language::markup::MarkupASTNode *, 8> NormalItems;
  auto Children = L->getChildren();
  if (Children.empty())
    return false;

  for (auto Child : Children) {
    auto I = dyn_cast<language::markup::Item>(Child);
    if (!I) {
      NormalItems.push_back(Child);
      continue;
    }

    auto ItemChildren = I->getChildren();
    if (ItemChildren.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    auto FirstChild = ItemChildren.front();
    auto FirstParagraph = dyn_cast<language::markup::Paragraph>(FirstChild);
    if (!FirstParagraph) {
      NormalItems.push_back(Child);
      continue;
    }

    auto FirstParagraphChildren = FirstParagraph->getChildren();
    if (FirstParagraphChildren.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    auto HeadingText
        = dyn_cast<language::markup::Text>(FirstParagraphChildren.front());
    if (!HeadingText) {
      NormalItems.push_back(Child);
      continue;
    }

    auto HeadingContent = HeadingText->getLiteralContent();
    if (!HeadingContent.rtrim().equals_insensitive("parameters:")) {
      NormalItems.push_back(Child);
      continue;
    }

    auto Rest = ArrayRef<language::markup::MarkupASTNode *>(
        ItemChildren.begin() + 1, ItemChildren.end());
    if (Rest.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    for (auto Child : Rest) {
      auto SubList = dyn_cast<language::markup::List>(Child);
      if (!SubList)
        continue;

      for (auto SubListChild : SubList->getChildren()) {
        auto Param = extractParamOutlineItem(MC, SubListChild);
        if (Param.has_value()) {
          ParamFields.push_back(Param.value());
        }
      }
    }
  }

  if (NormalItems.size() != Children.size()) {
    L->setChildren(NormalItems);
  }

  return NormalItems.empty();
}

bool extractSeparatedParams(
    language::markup::MarkupContext &MC, language::markup::List *L,
    SmallVectorImpl<language::markup::ParamField *> &ParamFields) {
  SmallVector<language::markup::MarkupASTNode *, 8> NormalItems;
  auto Children = L->getChildren();

  for (auto Child : Children) {
    auto I = dyn_cast<language::markup::Item>(Child);
    if (!I) {
      NormalItems.push_back(Child);
      continue;
    }

    auto ItemChildren = I->getChildren();
    if (ItemChildren.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    auto FirstChild = ItemChildren.front();
    auto FirstParagraph = dyn_cast<language::markup::Paragraph>(FirstChild);
    if (!FirstParagraph) {
      NormalItems.push_back(Child);
      continue;
    }

    auto FirstParagraphChildren = FirstParagraph->getChildren();
    if (FirstParagraphChildren.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    auto ParagraphText
        = dyn_cast<language::markup::Text>(FirstParagraphChildren.front());
    if (!ParagraphText) {
      NormalItems.push_back(Child);
      continue;
    }

    StringRef ParameterPrefix("parameter ");
    auto ParagraphContent = ParagraphText->getLiteralContent();
    auto PotentialMatch = ParagraphContent.substr(0, ParameterPrefix.size());

    if (!PotentialMatch.starts_with_insensitive(ParameterPrefix)) {
      NormalItems.push_back(Child);
      continue;
    }

    ParagraphContent = ParagraphContent.substr(ParameterPrefix.size());
    ParagraphText->setLiteralContent(ParagraphContent.ltrim());

    auto ParamField = extractParamOutlineItem(MC, I);
    if (ParamField.has_value())
      ParamFields.push_back(ParamField.value());
    else
      NormalItems.push_back(Child);
  }

  if (NormalItems.size() != Children.size())
    L->setChildren(NormalItems);

  return NormalItems.empty();
}

bool extractSimpleField(
    language::markup::MarkupContext &MC, language::markup::List *L,
    language::markup::CommentParts &Parts,
    SmallVectorImpl<const language::markup::MarkupASTNode *> &BodyNodes) {
  auto Children = L->getChildren();
  SmallVector<language::markup::MarkupASTNode *, 8> NormalItems;
  for (auto Child : Children) {
    auto I = dyn_cast<language::markup::Item>(Child);
    if (!I) {
      NormalItems.push_back(Child);
      continue;
    }

    auto ItemChildren = I->getChildren();
    if (ItemChildren.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    auto FirstParagraph
        = dyn_cast<language::markup::Paragraph>(ItemChildren.front());
    if (!FirstParagraph) {
      NormalItems.push_back(Child);
      continue;
    }

    auto ParagraphChildren = FirstParagraph->getChildren();
    if (ParagraphChildren.empty()) {
      NormalItems.push_back(Child);
      continue;
    }

    auto ParagraphText
        = dyn_cast<language::markup::Text>(ParagraphChildren.front());
    if (!ParagraphText) {
      NormalItems.push_back(Child);
      continue;
    }

    StringRef Tag;
    StringRef Remainder;
    std::tie(Tag, Remainder) = ParagraphText->getLiteralContent().split(':');
    Tag = Tag.ltrim().rtrim();
    Remainder = Remainder.ltrim();

    if (!language::markup::isAFieldTag(Tag)) {
      NormalItems.push_back(Child);
      continue;
    }

    ParagraphText->setLiteralContent(Remainder);
    auto Field = language::markup::createSimpleField(MC, Tag, ItemChildren);

    if (auto RF = dyn_cast<language::markup::ReturnsField>(Field)) {
      Parts.ReturnsField = RF;
    } else if (auto TF = dyn_cast<language::markup::ThrowsField>(Field)) {
      Parts.ThrowsField = TF;
    } else if (auto TF = dyn_cast<language::markup::TagField>(Field)) {
      toolchain::SmallString<64> Scratch;
      toolchain::raw_svector_ostream OS(Scratch);
      printInlinesUnder(TF, OS);
      Parts.Tags.insert(MC.allocateCopy(OS.str()));
    } else if (auto LKF = dyn_cast<markup::LocalizationKeyField>(Field)) {
      Parts.LocalizationKeyField = LKF;
    } else {
      BodyNodes.push_back(Field);
    }
  }

  if (NormalItems.size() != Children.size())
    L->setChildren(NormalItems);

  return NormalItems.empty();
}
} // namespace

void language::printBriefComment(RawComment RC, toolchain::raw_ostream &OS) {
  markup::MarkupContext MC;
  markup::LineList LL = MC.getLineList(RC);
  auto *markupDoc = markup::parseDocument(MC, LL);

  auto children = markupDoc->getChildren();
  if (children.empty())
    return;
  auto FirstParagraph = dyn_cast<language::markup::Paragraph>(children.front());
  if (!FirstParagraph)
    return;
  language::markup::printInlinesUnder(FirstParagraph, OS);
}

language::markup::CommentParts
language::extractCommentParts(language::markup::MarkupContext &MC,
                    language::markup::MarkupASTNode *Node) {

  language::markup::CommentParts Parts;
  auto Children = Node->getChildren();
  if (Children.empty())
    return Parts;

  auto FirstParagraph
      = dyn_cast<language::markup::Paragraph>(Node->getChildren().front());
  if (FirstParagraph)
    Parts.Brief = FirstParagraph;

  SmallVector<const language::markup::MarkupASTNode *, 4> BodyNodes;
  SmallVector<language::markup::ParamField *, 8> ParamFields;

  // Look for special top-level lists
  size_t StartOffset = FirstParagraph == nullptr ? 0 : 1;
  for (auto C = Children.begin() + StartOffset; C != Children.end(); ++C) {
    if (auto L = dyn_cast<language::markup::List>(*C)) {
      // Could be one of the following:
      // 1. A parameter outline:
      //    - Parameters:
      //      - x: ...
      //      - y: ...
      // 2. An exploded parameter list:
      //    - parameter x: ...
      //    - parameter y: ...
      // 3. Some other simple field, including "returns" (see SimpleFields.def)
      auto ListNowEmpty = extractParameterOutline(MC, L, ParamFields);
      ListNowEmpty |= extractSeparatedParams(MC, L, ParamFields);
      ListNowEmpty |= extractSimpleField(MC, L, Parts, BodyNodes);
      if (ListNowEmpty)
        continue; // This drops the empty list from the doc comment body.
    }
    BodyNodes.push_back(*C);
  }

  // Copy BodyNodes and ParamFields into the MarkupContext.
  Parts.BodyNodes = MC.allocateCopy(toolchain::ArrayRef(BodyNodes));
  Parts.ParamFields = MC.allocateCopy(toolchain::ArrayRef(ParamFields));

  for (auto Param : Parts.ParamFields) {
    auto ParamParts = extractCommentParts(MC, Param);
    Param->setParts(ParamParts);
  }

  return Parts;
}

DocComment *DocComment::create(const Decl *D, markup::MarkupContext &MC,
                               RawComment RC) {
  assert(!RC.isEmpty());
  language::markup::LineList LL = MC.getLineList(RC);
  auto *Doc = language::markup::parseDocument(MC, LL);
  auto Parts = extractCommentParts(MC, Doc);
  return new (MC) DocComment(D, Doc, Parts);
}

void DocComment::addInheritanceNote(language::markup::MarkupContext &MC,
                                    TypeDecl *base) {
  auto text = MC.allocateCopy("This documentation comment was inherited from ");
  auto name = MC.allocateCopy(base->getNameStr());
  auto period = MC.allocateCopy(".");
  auto paragraph = markup::Paragraph::create(MC, {
    markup::Text::create(MC, text),
    markup::Code::create(MC, name),
    markup::Text::create(MC, period)});

  auto note = markup::NoteField::create(MC, {paragraph});

  SmallVector<const markup::MarkupASTNode *, 8> BodyNodes{
    Parts.BodyNodes.begin(), Parts.BodyNodes.end()};
  BodyNodes.push_back(note);
  Parts.BodyNodes = MC.allocateCopy(toolchain::ArrayRef(BodyNodes));
}

DocComment *language::getSingleDocComment(language::markup::MarkupContext &MC,
                                       const Decl *D) {
  PrettyStackTraceDecl StackTrace("parsing comment for", D);

  auto RC = D->getRawComment();
  if (RC.isEmpty())
    return nullptr;
  return DocComment::create(D, MC, RC);
}

const Decl *Decl::getDocCommentProvidingDecl() const {
  return evaluateOrDefault(getASTContext().evaluator,
                           DocCommentProvidingDeclRequest{this}, nullptr);
}

StringRef Decl::getSemanticBriefComment() const {
  if (!canHaveComment())
    return StringRef();

  auto &eval = getASTContext().evaluator;
  return evaluateOrDefault(eval, SemanticBriefCommentRequest{this},
                           StringRef());
}
