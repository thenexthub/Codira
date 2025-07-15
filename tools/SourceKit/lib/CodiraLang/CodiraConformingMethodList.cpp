//===--- CodiraConformingMethodList.cpp ------------------------------------===//
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

#include "CodiraASTManager.h"
#include "CodiraEditorDiagConsumer.h"
#include "CodiraLangSupport.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/ConformingMethodList.h"
#include "language/IDETool/IDEInspectionInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"

using namespace SourceKit;
using namespace language;
using namespace ide;

static void
translateConformingMethodListOptions(OptionsDictionary &from,
                                     ConformingMethodList::Options &to) {
  // ConformingMethodList doesn't receive any options at this point.
}

static void
deliverResults(SourceKit::ConformingMethodListConsumer &SKConsumer,
               CancellableResult<ConformingMethodListResults> Result) {
  switch (Result.getKind()) {
  case CancellableResultKind::Success: {
    SKConsumer.setReusingASTContext(Result->DidReuseAST);

    if (!Result->Result) {
      // If we have no results, don't call SKConsumer.handleResult which causes
      // empty results to be delivered.
      break;
    }

    SmallString<512> SS;
    toolchain::raw_svector_ostream OS(SS);

    unsigned TypeNameBegin = SS.size();
    Result->Result->ExprType.print(OS);
    unsigned TypeNameLength = SS.size() - TypeNameBegin;

    unsigned TypeUSRBegin = SS.size();
    CodiraLangSupport::printTypeUSR(Result->Result->ExprType, OS);
    unsigned TypeUSRLength = SS.size() - TypeUSRBegin;

    struct MemberInfo {
      size_t DeclNameBegin = 0;
      size_t DeclNameLength = 0;
      size_t TypeNameBegin = 0;
      size_t TypeNameLength = 0;
      size_t TypeUSRBegin = 0;
      size_t TypeUSRLength = 0;
      size_t DescriptionBegin = 0;
      size_t DescriptionLength = 0;
      size_t SourceTextBegin = 0;
      size_t SourceTextLength = 0;
      StringRef BriefComment;

      MemberInfo() {}
    };
    SmallVector<MemberInfo, 8> Members;

    for (auto member : Result->Result->Members) {
      Members.emplace_back();
      auto &memberElem = Members.back();

      auto resultTy = cast<FuncDecl>(member)->getResultInterfaceType();
      resultTy = resultTy.subst(
        Result->Result->ExprType->getMemberSubstitutionMap(member));

      // Name.
      memberElem.DeclNameBegin = SS.size();
      member->getName().print(OS);
      memberElem.DeclNameLength = SS.size() - memberElem.DeclNameBegin;

      // Type name.
      memberElem.TypeNameBegin = SS.size();
      resultTy.print(OS);
      memberElem.TypeNameLength = SS.size() - memberElem.TypeNameBegin;

      // Type USR.
      memberElem.TypeUSRBegin = SS.size();
      CodiraLangSupport::printTypeUSR(resultTy, OS);
      memberElem.TypeUSRLength = SS.size() - memberElem.TypeUSRBegin;

      // Description.
      memberElem.DescriptionBegin = SS.size();
      CodiraLangSupport::printMemberDeclDescription(
          member, Result->Result->ExprType, /*usePlaceholder=*/false, OS);
      memberElem.DescriptionLength = SS.size() - memberElem.DescriptionBegin;

      // Sourcetext.
      memberElem.SourceTextBegin = SS.size();
      CodiraLangSupport::printMemberDeclDescription(
          member, Result->Result->ExprType, /*usePlaceholder=*/true, OS);
      memberElem.SourceTextLength = SS.size() - memberElem.SourceTextBegin;

      // DocBrief.
      auto MaybeClangNode = member->getClangNode();
      if (MaybeClangNode) {
        if (auto *D = MaybeClangNode.getAsDecl()) {
          const auto &ClangContext = D->getASTContext();
          if (const clang::RawComment *RC =
                  ClangContext.getRawCommentForAnyRedecl(D))
            memberElem.BriefComment = RC->getBriefText(ClangContext);
        }
      } else {
        memberElem.BriefComment = member->getSemanticBriefComment();
      }
    }

    SourceKit::ConformingMethodListResult SKResult;
    SmallVector<SourceKit::ConformingMethodListResult::Member, 8> SKMembers;

    for (auto info : Members) {
      StringRef Name(SS.begin() + info.DeclNameBegin, info.DeclNameLength);
      StringRef TypeName(SS.begin() + info.TypeNameBegin, info.TypeNameLength);
      StringRef TypeUSR(SS.begin() + info.TypeUSRBegin, info.TypeUSRLength);
      StringRef Description(SS.begin() + info.DescriptionBegin,
                            info.DescriptionLength);
      StringRef SourceText(SS.begin() + info.SourceTextBegin,
                           info.SourceTextLength);
      SKMembers.push_back({Name, TypeName, TypeUSR, Description, SourceText,
                           info.BriefComment});
    }

    SKResult.TypeName = StringRef(SS.begin() + TypeNameBegin, TypeNameLength);
    SKResult.TypeUSR = StringRef(SS.begin() + TypeUSRBegin, TypeUSRLength);
    SKResult.Members = SKMembers;

    SKConsumer.handleResult(SKResult);
    break;
  }
  case CancellableResultKind::Failure:
    SKConsumer.failed(Result.getError());
    break;
  case CancellableResultKind::Cancelled:
    SKConsumer.cancelled();
    break;
  }
}

void CodiraLangSupport::getConformingMethodList(
    toolchain::MemoryBuffer *UnresolvedInputFile, unsigned Offset,
    OptionsDictionary *optionsDict, ArrayRef<const char *> Args,
    ArrayRef<const char *> ExpectedTypeNames,
    SourceKitCancellationToken CancellationToken,
    SourceKit::ConformingMethodListConsumer &SKConsumer,
    std::optional<VFSOptions> vfsOptions) {
  std::string error;

  // FIXME: the use of None as primary file is to match the fact we do not read
  // the document contents using the editor documents infrastructure.
  auto fileSystem =
      getFileSystem(vfsOptions, /*primaryFile=*/std::nullopt, error);
  if (!fileSystem) {
    return SKConsumer.failed(error);
  }

  ConformingMethodList::Options options;
  if (optionsDict) {
    translateConformingMethodListOptions(*optionsDict, options);
  }

  performWithParamsToCompletionLikeOperation(
      UnresolvedInputFile, Offset, /*InsertCodeCompletionToken=*/true, Args,
      fileSystem, CancellationToken,
      [&](CancellableResult<CompletionLikeOperationParams> ParmsResult) {
        ParmsResult.mapAsync<ConformingMethodListResults>(
            [&](auto &Params, auto DeliverTransformed) {
              getIDEInspectionInstance()->conformingMethodList(
                  Params.Invocation, Args, fileSystem, Params.completionBuffer,
                  Offset, Params.DiagC, ExpectedTypeNames,
                  Params.CancellationFlag, DeliverTransformed);
            },
            [&](auto Result) { deliverResults(SKConsumer, Result); });
      });
}
