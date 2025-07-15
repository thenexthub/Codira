//===--- CodiraTypeContextInfo.cpp -----------------------------------------===//
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
#include "CodiraLangSupport.h"
#include "CodiraEditorDiagConsumer.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/TypeContextInfo.h"
#include "language/IDETool/IDEInspectionInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Comment.h"
#include "clang/AST/Decl.h"

using namespace SourceKit;
using namespace language;
using namespace ide;

static void translateTypeContextInfoOptions(OptionsDictionary &from,
                                            TypeContextInfo::Options &to) {
  // TypeContextInfo doesn't receive any options at this point.
}

static void deliverResults(SourceKit::TypeContextInfoConsumer &SKConsumer,
                           CancellableResult<TypeContextInfoResult> Result) {
  switch (Result.getKind()) {
  case CancellableResultKind::Success: {
    SKConsumer.setReusingASTContext(Result->DidReuseAST);

    for (auto &Item : Result->Results) {
      SmallString<512> SS;
      toolchain::raw_svector_ostream OS(SS);

      unsigned TypeNameBegin = SS.size();
      Item.ExpectedTy.print(OS);
      unsigned TypeNameLength = SS.size() - TypeNameBegin;

      unsigned TypeUSRBegin = SS.size();
      CodiraLangSupport::printTypeUSR(Item.ExpectedTy, OS);
      unsigned TypeUSRLength = SS.size() - TypeUSRBegin;

      struct MemberInfo {
        size_t DeclNameBegin = 0;
        size_t DeclNameLength = 0;
        size_t DescriptionBegin = 0;
        size_t DescriptionLength = 0;
        size_t SourceTextBegin = 0;
        size_t SourceTextLength = 0;
        StringRef BriefComment;

        MemberInfo() {}
      };
      SmallVector<MemberInfo, 8> ImplicitMembers;
      for (auto member : Item.ImplicitMembers) {
        ImplicitMembers.emplace_back();
        auto &memberElem = ImplicitMembers.back();

        // Name.
        memberElem.DeclNameBegin = SS.size();
        member->getName().print(OS);
        memberElem.DeclNameLength = SS.size() - memberElem.DeclNameBegin;

        // Description.
        memberElem.DescriptionBegin = SS.size();
        CodiraLangSupport::printMemberDeclDescription(
            member, Item.ExpectedTy, /*usePlaceholder=*/false, OS);
        memberElem.DescriptionLength = SS.size() - memberElem.DescriptionBegin;

        // Sourcetext.
        memberElem.SourceTextBegin = SS.size();
        CodiraLangSupport::printMemberDeclDescription(
            member, Item.ExpectedTy, /*usePlaceholder=*/true, OS);
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

      SourceKit::TypeContextInfoItem Info;
      SmallVector<SourceKit::TypeContextInfoItem::Member, 8> SKImplicitMembers;

      for (auto &info : ImplicitMembers) {
        StringRef Name(SS.begin() + info.DeclNameBegin, info.DeclNameLength);
        StringRef Description(SS.begin() + info.DescriptionBegin,
                              info.DescriptionLength);
        StringRef SourceText(SS.begin() + info.SourceTextBegin,
                             info.SourceTextLength);
        SKImplicitMembers.push_back(
            {Name, Description, SourceText, info.BriefComment});
      }

      Info.TypeName = StringRef(SS.begin() + TypeNameBegin, TypeNameLength);
      Info.TypeUSR = StringRef(SS.begin() + TypeUSRBegin, TypeUSRLength);
      Info.ImplicitMembers = SKImplicitMembers;

      SKConsumer.handleResult(Info);
    }
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

void CodiraLangSupport::getExpressionContextInfo(
    toolchain::MemoryBuffer *UnresolvedInputFile, unsigned Offset,
    OptionsDictionary *optionsDict, ArrayRef<const char *> Args,
    SourceKitCancellationToken CancellationToken,
    SourceKit::TypeContextInfoConsumer &SKConsumer,
    std::optional<VFSOptions> vfsOptions) {
  std::string error;

  TypeContextInfo::Options options;
  if (optionsDict) {
    translateTypeContextInfoOptions(*optionsDict, options);
  }

  // FIXME: the use of None as primary file is to match the fact we do not read
  // the document contents using the editor documents infrastructure.
  auto fileSystem =
      getFileSystem(vfsOptions, /*primaryFile=*/std::nullopt, error);
  if (!fileSystem) {
    return SKConsumer.failed(error);
  }

  performWithParamsToCompletionLikeOperation(
      UnresolvedInputFile, Offset, /*InsertCodeCompletionToken=*/true, Args,
      fileSystem, CancellationToken,
      [&](CancellableResult<CompletionLikeOperationParams> ParamsResult) {
        ParamsResult.mapAsync<TypeContextInfoResult>(
            [&](auto &CIParams, auto DeliverTransformed) {
              getIDEInspectionInstance()->typeContextInfo(
                  CIParams.Invocation, Args, fileSystem,
                  CIParams.completionBuffer, Offset, CIParams.DiagC,
                  CIParams.CancellationFlag, DeliverTransformed);
            },
            [&](auto Result) { deliverResults(SKConsumer, Result); });
      });
}
