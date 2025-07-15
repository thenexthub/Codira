//===----------------------------------------------------------------------===//
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

#include "language/AST/ASTPrinter.h"
#include "language/AST/Decl.h"
#include "language/AST/NameLookup.h"
#include "language/Basic/Defer.h"
#include "language/Basic/SourceManager.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/CommentConversion.h"
#include "language/IDE/Utils.h"
#include "language/Markup/XMLUtils.h"
#include "language/Subsystems.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclObjC.h"
#include "clang/Basic/Module.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Index/USRGeneration.h"
#include "clang/Lex/Lexer.h"
#include "clang/Basic/CharInfo.h"

#include "toolchain/Support/MemoryBuffer.h"

#include <numeric>

using namespace language;
using namespace language::ide;

std::optional<std::pair<unsigned, unsigned>>
language::ide::parseLineCol(StringRef LineCol) {
  unsigned Line, Col;
  size_t ColonIdx = LineCol.find(':');
  if (ColonIdx == StringRef::npos) {
    toolchain::errs() << "wrong pos format, it should be '<line>:<column>'\n";
    return std::nullopt;
  }
  if (LineCol.substr(0, ColonIdx).getAsInteger(10, Line)) {
    toolchain::errs() << "wrong pos format, it should be '<line>:<column>'\n";
    return std::nullopt;
  }
  if (LineCol.substr(ColonIdx+1).getAsInteger(10, Col)) {
    toolchain::errs() << "wrong pos format, it should be '<line>:<column>'\n";
    return std::nullopt;
  }

  if (Line == 0 || Col == 0) {
    toolchain::errs() << "wrong pos format, line/col should start from 1\n";
    return std::nullopt;
  }

  return std::make_pair(Line, Col);
}

void XMLEscapingPrinter::printText(StringRef Text) {
  language::markup::appendWithXMLEscaping(OS, Text);
}

void XMLEscapingPrinter::printXML(StringRef Text) {
  OS << Text;
}

void ResolvedRangeInfo::print(toolchain::raw_ostream &OS) const {
  OS << "<Kind>";
  switch (Kind) {
  case RangeKind::SingleExpression: OS << "SingleExpression"; break;
  case RangeKind::SingleDecl: OS << "SingleDecl"; break;
  case RangeKind::MultiTypeMemberDecl: OS << "MultiTypeMemberDecl"; break;
  case RangeKind::MultiStatement: OS << "MultiStatement"; break;
  case RangeKind::PartOfExpression: OS << "PartOfExpression"; break;
  case RangeKind::SingleStatement: OS << "SingleStatement"; break;
  case RangeKind::Invalid: OS << "Invalid"; break;
  }
  OS << "</Kind>\n";

  OS << "<Content>" << ContentRange.str() << "</Content>\n";

  if (auto Ty = getType()) {
    OS << "<Type>";
    Ty->print(OS);
    OS << "</Type>";
    switch(exit()) {
    case ExitState::Positive: OS << "<Exit>true</Exit>"; break;
    case ExitState::Unsure: OS << "<Exit>unsure</Exit>"; break;
    case ExitState::Negative: OS << "<Exit>false</Exit>"; break;
    }
    OS << "\n";
  }

  if (RangeContext) {
    OS << "<Context>";
    printContext(OS, RangeContext);
    OS << "</Context>\n";
  }

  if (CommonExprParent) {
    OS << "<Parent>";
    OS << Expr::getKindName(CommonExprParent->getKind());
    OS << "</Parent>\n";
  }

  if (!HasSingleEntry) {
    OS << "<Entry>Multi</Entry>\n";
  }

  if (UnhandledEffects.contains(EffectKind::Throws)) {
    OS << "<Error>Throwing</Error>\n";
  }
  if (UnhandledEffects.contains(EffectKind::Async)) {
    OS << "<Effect>Async</Effect>\n";
  }

  if (Orphan != OrphanKind::None) {
    OS << "<Orphan>";
    switch (Orphan) {
    case OrphanKind::Continue:
      OS << "Continue";
      break;
    case OrphanKind::Break:
      OS << "Break";
      break;
    case OrphanKind::None:
      toolchain_unreachable("cannot enter here.");
    }
    OS << "</Orphan>";
  }

  for (auto &VD : DeclaredDecls) {
    OS << "<Declared>" << VD.VD->getBaseName() << "</Declared>";
    OS << "<OutscopeReference>";
    if (VD.ReferredAfterRange)
      OS << "true";
    else
      OS << "false";
    OS << "</OutscopeReference>\n";
  }
  for (auto &RD : ReferencedDecls) {
    OS << "<Referenced>" << RD.VD->getBaseName() << "</Referenced>";
    OS << "<Type>";
    RD.Ty->print(OS);
    OS << "</Type>\n";
  }

  OS << "<ASTNodes>" << ContainedNodes.size() << "</ASTNodes>\n";
  OS << "<end>\n";
}

CharSourceRange ResolvedRangeInfo::
calculateContentRange(ArrayRef<Token> Tokens) {
  if (Tokens.empty())
    return CharSourceRange();
  auto StartTok = Tokens.front();
  auto EndTok = Tokens.back();
  auto StartLoc = StartTok.hasComment() ?
    StartTok.getCommentStart() : StartTok.getLoc();
  auto EndLoc = EndTok.getRange().getEnd();
  auto Length = static_cast<const char *>(EndLoc.getOpaquePointerValue()) -
                static_cast<const char *>(StartLoc.getOpaquePointerValue());
  return CharSourceRange(StartLoc, Length);
}

bool DeclaredDecl::operator==(const DeclaredDecl& Other) const {
  return VD == Other.VD;
}

ReturnInfo::
ReturnInfo(ASTContext &Ctx, ArrayRef<ReturnInfo> Branches):
    ReturnType(Ctx.TheErrorType.getPointer()), Exit(ExitState::Unsure) {
  std::set<TypeBase*> AllTypes;
  std::set<ExitState> AllExitStates;
  for (auto I : Branches) {
    AllTypes.insert(I.ReturnType);
    AllExitStates.insert(I.Exit);
  }
  if (AllTypes.size() == 1) {
    ReturnType = *AllTypes.begin();
  }
  if (AllExitStates.size() == 1) {
    Exit = *AllExitStates.begin();
  }
}

CharSourceRange CallArgInfo::getEntireCharRange(const SourceManager &SM) const {
  return CharSourceRange(SM, LabelRange.getStart(),
                         Lexer::getLocForEndOfToken(SM, ArgExp->getEndLoc()));
}

static Expr* getSingleNonImplicitChild(Expr *Parent) {
  // If this expr is non-implicit, we are done.
  if (!Parent->isImplicit())
    return Parent;

  // Collect all immediate children.
  toolchain::SmallVector<Expr*, 4> Children;
  Parent->forEachImmediateChildExpr([&](Expr *E) {
    Children.push_back(E);
    return E;
  });

  // If more than one children are found, we are not sure the non-implicit node.
  if (Children.size() != 1)
    return Parent;

  // Dig deeper if necessary.
  return getSingleNonImplicitChild(Children[0]);
}

std::vector<CallArgInfo> language::ide::
getCallArgInfo(SourceManager &SM, ArgumentList *Args, LabelRangeEndAt EndKind) {
  std::vector<CallArgInfo> InfoVec;
  auto *OriginalArgs = Args->getOriginalArgs();
  for (auto ElemIndex : indices(*OriginalArgs)) {
    auto *Elem = OriginalArgs->getExpr(ElemIndex);

    SourceLoc LabelStart(Elem->getStartLoc());
    SourceLoc LabelEnd(LabelStart);

    bool IsTrailingClosure = OriginalArgs->isTrailingClosureIndex(ElemIndex);
    auto NameLoc = OriginalArgs->getLabelLoc(ElemIndex);
    if (NameLoc.isValid()) {
      LabelStart = NameLoc;
      if (EndKind == LabelRangeEndAt::LabelNameOnly || IsTrailingClosure) {
        LabelEnd = Lexer::getLocForEndOfToken(SM, NameLoc);
      }
    }
    InfoVec.push_back({getSingleNonImplicitChild(Elem),
                       CharSourceRange(SM, LabelStart, LabelEnd),
                       IsTrailingClosure});
  }
  return InfoVec;
}

std::pair<std::vector<CharSourceRange>, std::optional<unsigned>>
language::ide::getCallArgLabelRanges(SourceManager &SM, ArgumentList *Args,
                                  LabelRangeEndAt EndKind) {
  std::vector<CharSourceRange> Ranges;
  auto InfoVec = getCallArgInfo(SM, Args, EndKind);

  std::optional<unsigned> FirstTrailing;
  auto I = std::find_if(InfoVec.begin(), InfoVec.end(), [](CallArgInfo &Info) {
    return Info.IsTrailingClosure;
  });
  if (I != InfoVec.end())
    FirstTrailing = std::distance(InfoVec.begin(), I);

  toolchain::transform(InfoVec, std::back_inserter(Ranges),
                  [](CallArgInfo &Info) { return Info.LabelRange; });
  return {Ranges, FirstTrailing};
}
