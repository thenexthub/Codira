//===--- CodiraCompletion.cpp ----------------------------------------------===//
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

#include "CodeCompletionOrganizer.h"
#include "CodiraASTManager.h"
#include "CodiraLangSupport.h"
#include "CodiraEditorDiagConsumer.h"
#include "SourceKit/Support/FileSystemProvider.h"
#include "SourceKit/Support/Logging.h"
#include "SourceKit/Support/UIdent.h"

#include "language/AST/ASTPrinter.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/IDE/CodeCompletionCache.h"
#include "language/IDE/CodeCompletionResultPrinter.h"
#include "language/IDETool/IDEInspectionInstance.h"

#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/MemoryBuffer.h"

using namespace SourceKit;
using namespace language;
using namespace ide;
using CodeCompletion::CodeCompletionView;
using CodeCompletion::CodeCompletionViewRef;
using CodeCompletion::Completion;
using CodeCompletion::NameToPopularityMap;
using ide::CodiraCompletionInfo;

static_assert(language::ide::CodeCompletionResult::MaxNumBytesToErase == 127,
              "custom array implementation for code completion results "
              "has this limit hardcoded");

namespace {
struct CodiraToSourceKitCompletionAdapter {
  static bool handleResult(SourceKit::CodeCompletionConsumer &consumer,
                           CodeCompletionResult *result,
                           bool annotatedDescription) {
    toolchain::SmallString<64> description;
    {
      toolchain::raw_svector_ostream OSS(description);
      // FIXME: The leading punctuation (e.g. "?." in an optional completion)
      // should really be part of a structured result description and clients
      // can
      // decide whether to display it or not. For now just include it in the
      // description only in the new code path.
      ide::printCodeCompletionResultDescription(*result, OSS,
                                                /*leadingPunctuation=*/false);
    }

    Completion extended(*result, description);
    return handleResult(consumer, &extended, /*leadingPunctuation=*/false,
                        /*legacyLiteralToKeyword=*/true, annotatedDescription);
  }

  static bool handleResult(SourceKit::CodeCompletionConsumer &consumer,
                           Completion *result, bool leadingPunctuation,
                           bool legacyLiteralToKeyword,
                           bool annotatedDescription);

  static void
  getResultAssociatedUSRs(ArrayRef<NullTerminatedStringRef> AssocUSRs,
                          raw_ostream &OS);
};

} // anonymous namespace

/// Returns completion context kind \c UIdent to report to the client.
/// For now, only returns "unresolved member" kind because others are unstable.
/// Returns invalid \c UIents other cases.
static UIdent getUIDForCodeCompletionKindToReport(CompletionKind kind) {
  switch (kind) {
  case CompletionKind::UnresolvedMember:
    return UIdent("source.lang.code.completion.unresolvedmember");
  default:
    return UIdent();
  }
}

static void languageCodeCompleteImpl(
    CodiraLangSupport &Lang, toolchain::MemoryBuffer *UnresolvedInputFile,
    unsigned Offset, ArrayRef<const char *> Args,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
    const CodeCompletion::Options &opts,
    SourceKitCancellationToken CancellationToken,
    toolchain::function_ref<void(CancellableResult<CodeCompleteResult>)> Callback) {
  assert(Callback && "Must provide callback to receive results");

  auto languageCache = Lang.getCodeCompletionCache(); // Pin the cache.
  ide::CodeCompletionContext CompletionContext(languageCache->getCache());
  CompletionContext.setAnnotateResult(opts.annotatedDescription);
  CompletionContext.setIncludeObjectLiterals(opts.includeObjectLiterals);
  CompletionContext.setAddInitsToTopLevel(opts.addInitsToTopLevel);
  CompletionContext.setAddCallWithNoDefaultArgs(opts.addCallWithNoDefaultArgs);

  Lang.performWithParamsToCompletionLikeOperation(
      UnresolvedInputFile, Offset, /*InsertCodeCompletionToken=*/true, Args,
      FileSystem, CancellationToken,
      [&](CancellableResult<CodiraLangSupport::CompletionLikeOperationParams>
              ParamsResult) {
        ParamsResult.mapAsync<CodeCompleteResult>(
            [&](auto &CIParams, auto DeliverTransformed) {
              Lang.getIDEInspectionInstance()->codeComplete(
                  CIParams.Invocation, Args, FileSystem,
                  CIParams.completionBuffer, Offset, CIParams.DiagC,
                  CompletionContext, CIParams.CancellationFlag,
                  DeliverTransformed);
            },
            Callback);
      });
}

static void translateCodeCompletionOptions(OptionsDictionary &from,
                                           CodeCompletion::Options &to,
                                           StringRef &filterText,
                                           unsigned &resultOffset,
                                           unsigned &maxResults);

static void
deliverCodeCompleteResults(SourceKit::CodeCompletionConsumer &SKConsumer,
                           const CodeCompletion::Options &CCOpts,
                           CancellableResult<CodeCompleteResult> Result) {
  switch (Result.getKind()) {
  case CancellableResultKind::Success: {
    auto kind = getUIDForCodeCompletionKindToReport(
        Result->Info.completionContext->CodeCompletionKind);
    if (kind.isValid())
      SKConsumer.setCompletionKind(kind);

    bool hasRequiredType = Result->Info.completionContext->typeContextKind ==
                           TypeContextKind::Required;
    ArrayRef<CodeCompletionResult *> Results = Result->ResultSink.Results;
    // If the results are sorted by name, this stores the sorted results, which
    // will be referenced by `Results`.
    std::vector<CodeCompletionResult *> SortedResultsStorage;
    if (CCOpts.sortByName) {
      SortedResultsStorage =
          CodeCompletionContext::sortCompletionResults(Results);
      Results = SortedResultsStorage;
    }
    // FIXME: this adhoc filtering should be configurable like it is in the
    // codeCompleteOpen path.
    for (auto *Result : Results) {
      if (Result->getKind() == CodeCompletionResultKind::Literal) {
        switch (Result->getLiteralKind()) {
        case CodeCompletionLiteralKind::NilLiteral:
        case CodeCompletionLiteralKind::BooleanLiteral:
          break;
        case CodeCompletionLiteralKind::ImageLiteral:
        case CodeCompletionLiteralKind::ColorLiteral:
          if (hasRequiredType &&
              Result->getExpectedTypeRelation() <
                  CodeCompletionResultTypeRelation::Convertible)
            continue;
          break;
        default:
          continue;
        }
      }
      if (!CodiraToSourceKitCompletionAdapter::handleResult(
              SKConsumer, Result, CCOpts.annotatedDescription))
        break;
    }

    SKConsumer.setReusingASTContext(
        Result->Info.completionContext->ReusingASTContext);
    SKConsumer.setAnnotatedTypename(
        Result->Info.completionContext->getAnnotateResult());
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

void CodiraLangSupport::codeComplete(
    toolchain::MemoryBuffer *UnresolvedInputFile, unsigned Offset,
    OptionsDictionary *options, SourceKit::CodeCompletionConsumer &SKConsumer,
    ArrayRef<const char *> Args, std::optional<VFSOptions> vfsOptions,
    SourceKitCancellationToken CancellationToken) {

  CodeCompletion::Options CCOpts;
  if (options) {
    StringRef filterText;
    unsigned resultOffset = 0;
    unsigned maxResults = 0;
    translateCodeCompletionOptions(*options, CCOpts, filterText, resultOffset,
                                   maxResults);
  }

  std::string error;
  // FIXME: the use of None as primary file is to match the fact we do not read
  // the document contents using the editor documents infrastructure.
  auto fileSystem =
      getFileSystem(vfsOptions, /*primaryFile=*/std::nullopt, error);
  if (!fileSystem) {
    return SKConsumer.failed(error);
  }

  languageCodeCompleteImpl(
      *this, UnresolvedInputFile, Offset, Args, fileSystem, CCOpts,
      CancellationToken, [&](CancellableResult<CodeCompleteResult> Result) {
        deliverCodeCompleteResults(SKConsumer, CCOpts, Result);
      });
}

static void getResultStructure(
    const CodeCompletion::CodiraResult &result, bool leadingPunctuation,
    CodeCompletionInfo::DescriptionStructure &structure,
    std::vector<CodeCompletionInfo::ParameterStructure> &parameters) {
  auto *CCStr = result.getCompletionString();
  auto FirstTextChunk = CCStr->getFirstTextChunkIndex(leadingPunctuation);

  if (!FirstTextChunk.has_value())
    return;

  bool isOperator = result.isOperator();

  auto chunks = CCStr->getChunks();
  using ChunkKind = CodeCompletionString::Chunk::ChunkKind;

  unsigned i = *FirstTextChunk;
  unsigned textSize = 0;

  // The result name.
  for (; i < chunks.size(); ++i) {
    auto C = chunks[i];
    if (C.is(ChunkKind::TypeAnnotation) ||
        C.is(ChunkKind::CallArgumentClosureType) ||
        C.is(ChunkKind::CallArgumentClosureExpr) ||
        C.is(ChunkKind::Whitespace))
      continue;

    if (C.is(ChunkKind::LeftParen) || C.is(ChunkKind::LeftBracket) ||
        C.is(ChunkKind::BraceStmtWithCursor) ||
        C.is(ChunkKind::CallArgumentBegin))
      break;

    if (C.hasText())
      textSize += C.getText().size();
  }

  structure.baseName.begin = 0;
  structure.baseName.end = textSize;

  // The parameters.
  for (; i < chunks.size(); ++i) {
    auto &C = chunks[i];
    if (C.is(ChunkKind::TypeAnnotation) ||
        C.is(ChunkKind::CallArgumentClosureType) ||
        C.is(ChunkKind::CallArgumentClosureExpr) ||
        C.is(ChunkKind::Whitespace))
      continue;

    if (C.is(ChunkKind::BraceStmtWithCursor))
      break;

    if (C.is(ChunkKind::CallArgumentBegin)) {
      CodeCompletionInfo::ParameterStructure param;

      ++i;
      bool inName = false;
      bool inAfterColon = false;
      for (; i < chunks.size(); ++i) {
        if (chunks[i].endsPreviousNestedGroup(C.getNestingLevel()))
          break;
        if (chunks[i].is(ChunkKind::CallArgumentClosureType) ||
            chunks[i].is(ChunkKind::CallArgumentClosureExpr))
          continue;
        if (isOperator && chunks[i].is(ChunkKind::CallArgumentType))
          continue;

        // Parameter name
        if (chunks[i].is(ChunkKind::CallArgumentName) ||
            chunks[i].is(ChunkKind::CallArgumentInternalName)) {
          param.name.begin = textSize;
          param.isLocalName =
              chunks[i].is(ChunkKind::CallArgumentInternalName);
          inName = true;
        }

        // Parameter type
        if (chunks[i].is(ChunkKind::CallArgumentType)) {
          unsigned start = textSize;
          unsigned prev = i - 1; // if i == 0, prev = ~0u.

          // Combine & for inout into the type name.
          if (prev != ~0u && chunks[prev].is(ChunkKind::Ampersand)) {
            start -= chunks[prev].getText().size();
            prev -= 1;
          }

          // Combine the whitespace after ':' into the type name.
          if (prev != ~0u && chunks[prev].is(ChunkKind::CallArgumentColon))
            start -= 1;

          param.afterColon.begin = start;
          inAfterColon = true;
          if (inName) {
            param.name.end = start;
            inName = false;
          }
        }

        if (chunks[i].hasText())
          textSize += chunks[i].getText().size();
      }

      // If we had a name with no type following it, finish it now.
      if (inName)
        param.name.end = textSize;

      // Finish the type name.
      if (inAfterColon)
        param.afterColon.end = textSize;
      if (!param.range().empty())
        parameters.push_back(std::move(param));

      if (i < chunks.size() && chunks[i].hasText())
        textSize += chunks[i].getText().size();
    }

    if (C.hasText())
      textSize += C.getText().size();
  }

  if (!parameters.empty()) {
    structure.parameterRange.begin = parameters.front().range().begin;
    structure.parameterRange.end = parameters.back().range().end;
  }
}

static UIdent KindLiteralArray("source.lang.code.literal.array");
static UIdent KindLiteralBoolean("source.lang.code.literal.boolean");
static UIdent KindLiteralColor("source.lang.code.literal.color");
static UIdent KindLiteralImage("source.lang.code.literal.image");
static UIdent KindLiteralDictionary("source.lang.code.literal.dictionary");
static UIdent KindLiteralInteger("source.lang.code.literal.integer");
static UIdent KindLiteralNil("source.lang.code.literal.nil");
static UIdent KindLiteralString("source.lang.code.literal.string");
static UIdent KindLiteralTuple("source.lang.code.literal.tuple");

static UIdent
getUIDForCodeCompletionLiteralKind(CodeCompletionLiteralKind kind) {
  switch (kind) {
  case CodeCompletionLiteralKind::ArrayLiteral:
    return KindLiteralArray;
  case CodeCompletionLiteralKind::BooleanLiteral:
    return KindLiteralBoolean;
  case CodeCompletionLiteralKind::ColorLiteral:
    return KindLiteralColor;
  case CodeCompletionLiteralKind::ImageLiteral:
    return KindLiteralImage;
  case CodeCompletionLiteralKind::DictionaryLiteral:
    return KindLiteralDictionary;
  case CodeCompletionLiteralKind::IntegerLiteral:
    return KindLiteralInteger;
  case CodeCompletionLiteralKind::NilLiteral:
    return KindLiteralNil;
  case CodeCompletionLiteralKind::StringLiteral:
    return KindLiteralString;
  case CodeCompletionLiteralKind::Tuple:
    return KindLiteralTuple;
  }
}

static UIdent KeywordUID("source.lang.code.keyword");
static UIdent KeywordLetUID("source.lang.code.keyword.let");
static UIdent KeywordVarUID("source.lang.code.keyword.var");
static UIdent KeywordIfUID("source.lang.code.keyword.if");
static UIdent KeywordForUID("source.lang.code.keyword.for");
static UIdent KeywordWhileUID("source.lang.code.keyword.while");
static UIdent KeywordFuncUID("source.lang.code.keyword.fn");

bool CodiraToSourceKitCompletionAdapter::handleResult(
    SourceKit::CodeCompletionConsumer &Consumer, Completion *Result,
    bool leadingPunctuation, bool legacyLiteralToKeyword,
    bool annotatedDescription) {

  static UIdent KeywordUID("source.lang.code.keyword");
  static UIdent PatternUID("source.lang.code.pattern");

  CodeCompletionInfo Info;
  if (Result->hasCustomKind()) {
    Info.CustomKind = Result->getCustomKind();
  } else if (Result->getKind() == CodeCompletionResultKind::Keyword) {
    Info.Kind = KeywordUID;
  } else if (Result->getKind() == CodeCompletionResultKind::Pattern) {
    Info.Kind = PatternUID;
  } else if (Result->getKind() == CodeCompletionResultKind::BuiltinOperator) {
    Info.Kind = PatternUID; // FIXME: add a UID for operators
  } else if (Result->getKind() == CodeCompletionResultKind::Declaration) {
    Info.Kind = CodiraLangSupport::getUIDForCodeCompletionDeclKind(
        Result->getAssociatedDeclKind());
  } else if (Result->getKind() == CodeCompletionResultKind::Literal) {
    auto literalKind = Result->getLiteralKind();
    if (legacyLiteralToKeyword &&
        (literalKind == CodeCompletionLiteralKind::BooleanLiteral ||
         literalKind == CodeCompletionLiteralKind::NilLiteral)) {
      // Fallback to keywords as appropriate.
      Info.Kind = KeywordUID;
    } else {
      Info.Kind = getUIDForCodeCompletionLiteralKind(literalKind);
    }
  }

  if (Info.Kind.isInvalid() && !Info.CustomKind)
    return true;

  toolchain::SmallString<512> SS;

  unsigned DescBegin = SS.size();
  {
    toolchain::raw_svector_ostream ccOS(SS);
    if (annotatedDescription)
      ide::printCodeCompletionResultDescriptionAnnotated(*Result, ccOS,
                                                         leadingPunctuation);
    else
      ide::printCodeCompletionResultDescription(*Result, ccOS,
                                                leadingPunctuation);
  }
  unsigned DescEnd = SS.size();

  if (DescBegin == DescEnd) {
    LOG_FUNC_SECTION_WARN {
      toolchain::SmallString<64> LogMessage;
      toolchain::raw_svector_ostream LogMessageOs(LogMessage);

      LogMessageOs << "Code completion result with empty description "
                      "was ignored: \n";
      Result->getCodiraResult().printPrefix(LogMessageOs);
      Result->getCompletionString()->print(LogMessageOs);

      *Log << LogMessage;
    }
    return true;
  }

  unsigned TextBegin = SS.size();
  {
    toolchain::raw_svector_ostream ccOS(SS);
    ide::printCodeCompletionResultSourceText(*Result, ccOS);
  }
  unsigned TextEnd = SS.size();

  unsigned TypeBegin = SS.size();
  {
    toolchain::raw_svector_ostream ccOS(SS);
    if (annotatedDescription)
      ide::printCodeCompletionResultTypeNameAnnotated(*Result, ccOS);
    else
      ide::printCodeCompletionResultTypeName(*Result, ccOS);
  }
  unsigned TypeEnd = SS.size();

  unsigned USRsBegin = SS.size();
  {
    toolchain::raw_svector_ostream ccOS(SS);
    getResultAssociatedUSRs(Result->getAssociatedUSRs(), ccOS);
  }
  unsigned USRsEnd = SS.size();

  Info.Name = Result->getFilterName();
  Info.Description = StringRef(SS.begin() + DescBegin, DescEnd - DescBegin);
  Info.SourceText = StringRef(SS.begin() + TextBegin, TextEnd - TextBegin);
  Info.TypeName = StringRef(SS.begin() + TypeBegin, TypeEnd - TypeBegin);
  Info.AssocUSRs = StringRef(SS.begin() + USRsBegin, USRsEnd - USRsBegin);

  static UIdent CCCtxNone("source.codecompletion.context.none");
  static UIdent CCCtxExpressionSpecific(
      "source.codecompletion.context.exprspecific");
  static UIdent CCCtxLocal("source.codecompletion.context.local");
  static UIdent CCCtxCurrentNominal("source.codecompletion.context.thisclass");
  static UIdent CCCtxSuper("source.codecompletion.context.superclass");
  static UIdent CCCtxOutsideNominal("source.codecompletion.context.otherclass");
  static UIdent CCCtxCurrentModule("source.codecompletion.context.thismodule");
  static UIdent CCCtxOtherModule("source.codecompletion.context.othermodule");


  if (Result->getFlair().contains(CodeCompletionFlairBit::ExpressionSpecific) ||
      Result->getFlair().contains(CodeCompletionFlairBit::SuperChain)) {
    // NOTE: `CCCtxExpressionSpecific` is to maintain compatibility.
    Info.SemanticContext = CCCtxExpressionSpecific;
  } else {
    switch (Result->getSemanticContext()) {
    case SemanticContextKind::None:
      Info.SemanticContext = CCCtxNone; break;
    case SemanticContextKind::Local:
      Info.SemanticContext = CCCtxLocal; break;
    case SemanticContextKind::CurrentNominal:
      Info.SemanticContext = CCCtxCurrentNominal; break;
    case SemanticContextKind::Super:
      Info.SemanticContext = CCCtxSuper; break;
    case SemanticContextKind::OutsideNominal:
      Info.SemanticContext = CCCtxOutsideNominal; break;
    case SemanticContextKind::CurrentModule:
      Info.SemanticContext = CCCtxCurrentModule; break;
    case SemanticContextKind::OtherModule:
      Info.SemanticContext = CCCtxOtherModule; break;
    }
  }

  static UIdent CCTypeRelNotApplicable("source.codecompletion.typerelation.notapplicable");
  static UIdent CCTypeRelUnknown("source.codecompletion.typerelation.unknown");
  static UIdent CCTypeRelUnrelated("source.codecompletion.typerelation.unrelated");
  static UIdent CCTypeRelInvalid("source.codecompletion.typerelation.invalid");
  static UIdent CCTypeRelConvertible("source.codecompletion.typerelation.convertible");

  switch (Result->getExpectedTypeRelation()) {
  case CodeCompletionResultTypeRelation::NotApplicable:
    Info.TypeRelation = CCTypeRelNotApplicable; break;
  case CodeCompletionResultTypeRelation::Unknown:
    Info.TypeRelation = CCTypeRelUnknown; break;
  case CodeCompletionResultTypeRelation::Unrelated:
    Info.TypeRelation = CCTypeRelUnrelated; break;
  case CodeCompletionResultTypeRelation::Invalid:
    Info.TypeRelation = CCTypeRelInvalid; break;
  case CodeCompletionResultTypeRelation::Convertible:
    Info.TypeRelation = CCTypeRelConvertible; break;
  }

  Info.ModuleName = Result->getModuleName();
  Info.DocBrief = Result->getBriefDocComment();
  Info.NotRecommended = Result->isNotRecommended();
  Info.IsSystem = Result->isSystem();

  Info.NumBytesToErase = Result->getNumBytesToErase();

  // Extended result values.
  Info.ModuleImportDepth = Result->getModuleImportDepth();

  // Description structure.
  std::vector<CodeCompletionInfo::ParameterStructure> parameters;
  CodeCompletionInfo::DescriptionStructure structure;
  getResultStructure(*Result, leadingPunctuation, structure, parameters);
  Info.descriptionStructure = structure;
  if (!parameters.empty())
    Info.parametersStructure = parameters;

  return Consumer.handleResult(Info);
}

static CodeCompletionLiteralKind
getCodeCompletionLiteralKindForUID(UIdent uid) {
  if (uid == KindLiteralArray) {
    return CodeCompletionLiteralKind::ArrayLiteral;
  } else if (uid == KindLiteralBoolean) {
    return CodeCompletionLiteralKind::BooleanLiteral;
  } else if (uid == KindLiteralColor) {
    return CodeCompletionLiteralKind::ColorLiteral;
  } else if (uid == KindLiteralImage) {
    return CodeCompletionLiteralKind::ImageLiteral;
  } else if (uid == KindLiteralDictionary) {
    return CodeCompletionLiteralKind::DictionaryLiteral;
  } else if (uid == KindLiteralInteger) {
    return CodeCompletionLiteralKind::IntegerLiteral;
  } else if (uid == KindLiteralNil) {
    return CodeCompletionLiteralKind::NilLiteral;
  } else if (uid == KindLiteralString) {
    return CodeCompletionLiteralKind::StringLiteral;
  } else if (uid == KindLiteralTuple) {
    return CodeCompletionLiteralKind::Tuple;
  } else {
    toolchain_unreachable("unexpected literal kind");
  }
}

static CodeCompletionKeywordKind
getCodeCompletionKeywordKindForUID(UIdent uid) {
#define LANGUAGE_KEYWORD(kw) \
  static UIdent Keyword##kw##UID("source.lang.code.keyword." #kw); \
  if (uid == Keyword##kw##UID) { \
    return CodeCompletionKeywordKind::kw_##kw; \
  }
#include "language/AST/TokenKinds.def"

  // FIXME: should warn about unexpected keyword kind.
  return CodeCompletionKeywordKind::None;
}

void CodiraToSourceKitCompletionAdapter::getResultAssociatedUSRs(
    ArrayRef<NullTerminatedStringRef> AssocUSRs, raw_ostream &OS) {
  bool First = true;
  for (auto USR : AssocUSRs) {
    if (!First)
      OS << " ";
    First = false;
    OS << USR;
  }
}

//===----------------------------------------------------------------------===//
// CodeCompletion::SessionCache
//===----------------------------------------------------------------------===//
void CodeCompletion::SessionCache::setSortedCompletions(
    std::vector<Completion *> &&completions) {
  toolchain::sys::ScopedLock L(mtx);
  sortedCompletions = std::move(completions);
}
ArrayRef<Completion *> CodeCompletion::SessionCache::getSortedCompletions() {
  toolchain::sys::ScopedLock L(mtx);
  return sortedCompletions;
}
toolchain::MemoryBuffer *CodeCompletion::SessionCache::getBuffer() {
  toolchain::sys::ScopedLock L(mtx);
  return buffer.get();
}
ArrayRef<std::string> CodeCompletion::SessionCache::getCompilerArgs() {
  toolchain::sys::ScopedLock L(mtx);
  return args;
}
toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> CodeCompletion::SessionCache::getFileSystem() {
  toolchain::sys::ScopedLock L(mtx);
  return fileSystem;
}
CompletionKind CodeCompletion::SessionCache::getCompletionKind() {
  toolchain::sys::ScopedLock L(mtx);
  return completionKind;
}
TypeContextKind CodeCompletion::SessionCache::getCompletionTypeContextKind() {
  toolchain::sys::ScopedLock L(mtx);
  return typeContextKind;
}
bool CodeCompletion::SessionCache::getCompletionMayUseImplicitMemberExpr() {
  toolchain::sys::ScopedLock L(mtx);
  return completionMayUseImplicitMemberExpr;
}
const CodeCompletion::FilterRules &
CodeCompletion::SessionCache::getFilterRules() {
  toolchain::sys::ScopedLock L(mtx);
  return filterRules;
}

//===----------------------------------------------------------------------===//
// CodeCompletion::SessionCacheMap
//===----------------------------------------------------------------------===//

unsigned CodeCompletion::SessionCacheMap::getBufferID(StringRef name) const {
  auto pair = nameToBufferMap.insert(std::make_pair(name, nextBufferID));
  if (pair.second)
    ++nextBufferID;
  return pair.first->getValue();
}

CodeCompletion::SessionCacheRef
CodeCompletion::SessionCacheMap::get(StringRef name, unsigned offset) const {
  toolchain::sys::ScopedLock L(mtx);
  auto key = std::make_pair(getBufferID(name), offset);
  auto I = sessions.find(key);
  if (I == sessions.end())
    return nullptr;
  return I->second;
}
bool CodeCompletion::SessionCacheMap::set(StringRef name, unsigned offset,
                                          CodeCompletion::SessionCacheRef s) {
  toolchain::sys::ScopedLock L(mtx);
  auto key = std::make_pair(getBufferID(name), offset);
  return sessions.insert(std::make_pair(key, s)).second;
}
bool CodeCompletion::SessionCacheMap::remove(StringRef name, unsigned offset) {
  toolchain::sys::ScopedLock L(mtx);
  auto key = std::make_pair(getBufferID(name), offset);
  return sessions.erase(key);
}

//===----------------------------------------------------------------------===//
// (New) Code completion interface
//===----------------------------------------------------------------------===//

namespace {
class CodiraGroupedCodeCompletionConsumer : public CodeCompletionView::Walker {
  GroupedCodeCompletionConsumer &consumer;

public:
  CodiraGroupedCodeCompletionConsumer(GroupedCodeCompletionConsumer &consumer)
      : consumer(consumer) {}
  bool handleResult(Completion *result) override {
    return CodiraToSourceKitCompletionAdapter::handleResult(
        consumer, result, /*leadingPunctuation=*/true,
        /*legacyLiteralToKeyword=*/false, /*annotatedDescription=*/false);
  }
  void startGroup(StringRef name) override {
    static UIdent GroupUID("source.lang.code.codecomplete.group");
    consumer.startGroup(GroupUID, name);
  }
  void endGroup() override { consumer.endGroup(); }
};

} // end anonymous namespace

static void translateCodeCompletionOptions(OptionsDictionary &from,
                                           CodeCompletion::Options &to,
                                           StringRef &filterText,
                                           unsigned &resultOffset,
                                           unsigned &maxResults) {
  static UIdent KeySortByName("key.codecomplete.sort.byname");
  static UIdent KeyUseImportDepth("key.codecomplete.sort.useimportdepth");
  static UIdent KeyGroupOverloads("key.codecomplete.group.overloads");
  static UIdent KeyGroupStems("key.codecomplete.group.stems");
  static UIdent KeyFilterText("key.codecomplete.filtertext");
  static UIdent KeyRequestLimit("key.codecomplete.requestlimit");
  static UIdent KeyRequestStart("key.codecomplete.requeststart");
  static UIdent KeyHideUnderscores("key.codecomplete.hideunderscores");
  static UIdent KeyHideLowPriority("key.codecomplete.hidelowpriority");
  static UIdent KeyHideByName("key.codecomplete.hidebyname");
  static UIdent KeyIncludeExactMatch("key.codecomplete.includeexactmatch");
  static UIdent KeyAddInnerResults("key.codecomplete.addinnerresults");
  static UIdent KeyAddInnerOperators("key.codecomplete.addinneroperators");
  static UIdent KeyAddInitsToTopLevel("key.codecomplete.addinitstotoplevel");
  static UIdent KeyFuzzyMatching("key.codecomplete.fuzzymatching");
  static UIdent KeyTopNonLiteral("key.codecomplete.showtopnonliteralresults");
  static UIdent KeyContextWeight("key.codecomplete.sort.contextweight");
  static UIdent KeyFuzzyWeight("key.codecomplete.sort.fuzzyweight");
  static UIdent KeyPopularityBonus("key.codecomplete.sort.popularitybonus");
  static UIdent KeyAnnotatedDescription("key.codecomplete.annotateddescription");
  static UIdent KeyIncludeObjectLiterals("key.codecomplete.includeobjectliterals");

  from.valueForOption(KeySortByName, to.sortByName);
  from.valueForOption(KeyUseImportDepth, to.useImportDepth);
  from.valueForOption(KeyGroupOverloads, to.groupOverloads);
  from.valueForOption(KeyGroupStems, to.groupStems);
  from.valueForOption(KeyFilterText, filterText);
  from.valueForOption(KeyRequestLimit, maxResults);
  from.valueForOption(KeyRequestStart, resultOffset);
  unsigned howMuchHiding = 1;
  from.valueForOption(KeyHideUnderscores, howMuchHiding);
  to.hideUnderscores = howMuchHiding;
  to.reallyHideAllUnderscores = howMuchHiding > 1;
  from.valueForOption(KeyHideLowPriority, to.hideLowPriority);
  from.valueForOption(KeyIncludeExactMatch, to.includeExactMatch);
  from.valueForOption(KeyAddInnerResults, to.addInnerResults);
  from.valueForOption(KeyAddInnerOperators, to.addInnerOperators);
  from.valueForOption(KeyAddInitsToTopLevel, to.addInitsToTopLevel);
  from.valueForOption(KeyFuzzyMatching, to.fuzzyMatching);
  from.valueForOption(KeyContextWeight, to.semanticContextWeight);
  from.valueForOption(KeyFuzzyWeight, to.fuzzyMatchWeight);
  from.valueForOption(KeyPopularityBonus, to.popularityBonus);
  from.valueForOption(KeyHideByName, to.hideByNameStyle);
  from.valueForOption(KeyTopNonLiteral, to.showTopNonLiteralResults);
  from.valueForOption(KeyAnnotatedDescription, to.annotatedDescription);
  from.valueForOption(KeyIncludeObjectLiterals, to.includeObjectLiterals);
}

/// Canonicalize a name that is in the format of a reference to a function into
/// the name format used internally for filtering.
///
/// Returns true if the name is invalid.
static bool canonicalizeFilterName(const char *origName,
                                   SmallVectorImpl<char> &Result) {
  assert(origName);
  const char *p = origName;
  char curr = '\0';
  char prev;

  // FIXME: disallow unnamed parameters without underscores `foo(::)`.
  while (true) {
    prev = curr;
    curr = *p++;
    switch (curr) {
    case '\0':
      return false; // Done.
    case '_':
      // Remove the _ underscore for an unnamed parameter.
      if (prev == ':' || prev == '(') {
        char next = *p;
        if (next == ':' || next == ')')
          continue;
      }
      TOOLCHAIN_FALLTHROUGH;
    default:
      Result.push_back(curr);
      continue;
    }
  }
}

static void translateFilterRules(ArrayRef<FilterRule> rawFilterRules,
                                 CodeCompletion::FilterRules &filterRules) {
  for (auto &rule : rawFilterRules) {
    switch (rule.kind) {
    case FilterRule::Everything:
      filterRules.hideAll = rule.hide;
      break;
    case FilterRule::Identifier:
      for (auto name : rule.names) {
        SmallString<128> canonName;
        // Note: name is null-terminated.
        if (canonicalizeFilterName(name.data(), canonName))
          continue;
        filterRules.hideByFilterName[canonName] = rule.hide;
      }
      break;
    case FilterRule::Description:
      for (auto name : rule.names) {
        filterRules.hideByDescription[name] = rule.hide;
      }
      break;
    case FilterRule::Module:
      for (auto name : rule.names) {
        filterRules.hideModule[name] = rule.hide;
      }
      break;
    case FilterRule::Keyword:
      if (rule.uids.empty())
        filterRules.hideAllKeywords = rule.hide;
      for (auto uid : rule.uids) {
        auto kind = getCodeCompletionKeywordKindForUID(uid);
        filterRules.hideKeyword[kind] = rule.hide;
      }
      break;
    case FilterRule::Literal:
      if (rule.uids.empty())
        filterRules.hideAllValueLiterals = rule.hide;
      for (auto uid : rule.uids) {
        auto kind = getCodeCompletionLiteralKindForUID(uid);
        filterRules.hideValueLiteral[kind] = rule.hide;
      }
      break;
    case FilterRule::CustomCompletion:
      if (rule.uids.empty())
        filterRules.hideCustomCompletions = rule.hide;
      // FIXME: hide individual custom completions
      break;
    }
  }
}

static bool checkInnerResult(const CodeCompletionResult &result, bool &hasDot,
                             bool &hasQDot, bool &hasInit) {
  auto chunks = result.getCompletionString()->getChunks();
  if (!chunks.empty() &&
      chunks[0].is(CodeCompletionString::Chunk::ChunkKind::Dot)) {
    hasDot = true;
    return true;
  } else if (chunks.size() >= 2 &&
             chunks[0].is(
                 CodeCompletionString::Chunk::ChunkKind::QuestionMark) &&
             chunks[1].is(CodeCompletionString::Chunk::ChunkKind::Dot)) {
    hasQDot = true;
    return true;
  } else if (result.getKind() == CodeCompletionResultKind::Declaration &&
             result.getAssociatedDeclKind() ==
                 CodeCompletionDeclKind::Constructor) {
    hasInit = true;
    return true;
  } else {
    return false;
  }
}

template <typename Result>
static std::vector<Result *>
filterInnerResults(ArrayRef<Result *> results, bool includeInner,
                   bool includeInnerOperators,
                   bool &hasDot, bool &hasQDot, bool &hasInit,
                   const CodeCompletion::FilterRules &rules) {
  std::vector<Result *> topResults;
  for (Result *result : results) {
    if (!includeInnerOperators && result->isOperator())
      continue;

    toolchain::SmallString<64> description;
    {
      toolchain::raw_svector_ostream OSS(description);
      ide::printCodeCompletionResultDescription(*result, OSS,
                                                /*leadingPunctuation=*/false);
    }
    if (rules.hideCompletion(*result, result->getFilterName(), description))
      continue;

    bool inner = checkInnerResult(*result, hasDot, hasQDot, hasInit);

    if (!inner ||
        (includeInner &&
         result->getSemanticContext() <= SemanticContextKind::CurrentNominal))
      topResults.push_back(result);
  }
  return topResults;
}

static void transformAndForwardResults(
    GroupedCodeCompletionConsumer &consumer, CodiraLangSupport &lang,
    CodeCompletion::SessionCacheRef session,
    const NameToPopularityMap *nameToPopularity,
    CodeCompletion::Options options, unsigned offset, StringRef filterText,
    unsigned resultOffset, unsigned maxResults,
    SourceKitCancellationToken CancellationToken) {

  CodeCompletion::CompletionSink innerSink;
  Completion *exactMatch = nullptr;
  auto buildInnerResult = [&](ArrayRef<CodeCompletionString::Chunk> chunks) {
    auto *completionString =
        CodeCompletionString::create(innerSink.allocator, chunks);
    ContextFreeCodeCompletionResult *contextFreeResult =
        ContextFreeCodeCompletionResult::createPatternOrBuiltInOperatorResult(
            innerSink.codeSink, CodeCompletionResultKind::BuiltinOperator,
            completionString, CodeCompletionOperatorKind::None,
            /*BriefDocComment=*/"", CodeCompletionResultType::notApplicable(),
            ContextFreeNotRecommendedReason::None,
            CodeCompletionDiagnosticSeverity::None,
            /*DiagnosticMessage=*/"");
    auto *paren = new (innerSink.allocator) CodeCompletion::CodiraResult(
        *contextFreeResult, SemanticContextKind::CurrentNominal,
        CodeCompletionFlairBit::ExpressionSpecific,
        exactMatch ? exactMatch->getNumBytesToErase() : 0,
        CodeCompletionResultTypeRelation::Unrelated,
        ContextualNotRecommendedReason::None);

    CodiraCompletionInfo info;
    std::vector<Completion *> extended = extendCompletions(
        paren, innerSink, info, nameToPopularity, options, exactMatch);
    assert(extended.size() == 1);
    return extended.front();
  };
  auto buildParen = [&]() {
    return buildInnerResult(CodeCompletionString::Chunk::createWithText(
        CodeCompletionString::Chunk::ChunkKind::LeftParen, 0, "("));
  };
  auto buildDot = [&]() {
    return buildInnerResult(CodeCompletionString::Chunk::createWithText(
        CodeCompletionString::Chunk::ChunkKind::Dot, 0, "."));
  };
  auto buildQDot = [&]() {
    CodeCompletionString::Chunk chunks[2] = {
        CodeCompletionString::Chunk::createWithText(
            CodeCompletionString::Chunk::ChunkKind::QuestionMark, 0, "?"),
        CodeCompletionString::Chunk::createWithText(
            CodeCompletionString::Chunk::ChunkKind::Dot, 0, ".")};
    return buildInnerResult(chunks);
  };

  CodeCompletion::CodeCompletionOrganizer organizer(
      options, session->getCompletionKind(),
      session->getCompletionTypeContextKind());

  auto &rules = session->getFilterRules();

  bool hasEarlyInnerResults =
      session->getCompletionKind() == CompletionKind::PostfixExpr;

  if (!hasEarlyInnerResults) {
    organizer.addCompletionsWithFilter(session->getSortedCompletions(),
                                       filterText, rules, exactMatch);
    // Add leading dot?
    if (options.addInnerOperators && !rules.hideFilterName(".") &&
        session->getCompletionMayUseImplicitMemberExpr()) {
      organizer.addCompletionsWithFilter(
          buildDot(), filterText, CodeCompletion::FilterRules(), exactMatch);
    }
  }

  if (hasEarlyInnerResults &&
      (options.addInnerResults || options.addInnerOperators)) {
    bool hasInit = false;
    bool hasDot = false;
    bool hasQDot = false;
    auto completions = session->getSortedCompletions();
    auto innerResults =
        filterInnerResults(completions, options.addInnerResults,
                           options.addInnerOperators, hasDot, hasQDot, hasInit,
                           rules);
    if (options.addInnerOperators) {
      if (hasInit && !rules.hideFilterName("("))
        innerResults.insert(innerResults.begin(), buildParen());
      if (hasDot && !rules.hideFilterName("."))
        innerResults.insert(innerResults.begin(), buildDot());
      if (hasQDot && !rules.hideFilterName("?."))
        innerResults.insert(innerResults.begin(), buildQDot());
    }

    organizer.addCompletionsWithFilter(innerResults, filterText,
                                       CodeCompletion::FilterRules(), exactMatch);
  }

  organizer.groupAndSort(options);

  if ((options.addInnerResults || options.addInnerOperators) && exactMatch &&
      exactMatch->getKind() == CodeCompletionResultKind::Declaration) {
    std::vector<Completion *> innerResults;
    bool hasDot = false;
    bool hasQDot = false;
    bool hasInit = false;

    auto *inputBuf = session->getBuffer();
    std::string str = inputBuf->getBuffer().slice(0, offset).str();
    {
      toolchain::raw_string_ostream OSS(str);
      ide::printCodeCompletionResultSourceText(*exactMatch, OSS);
    }

    auto buffer =
        toolchain::MemoryBuffer::getMemBuffer(str, inputBuf->getBufferIdentifier());
    auto args = session->getCompilerArgs();
    std::vector<const char *> cargs;
    for (auto &arg : args)
      cargs.push_back(arg.c_str());

    bool CodeCompleteDidFail = false;
    bool CallbackCalled = false;

    languageCodeCompleteImpl(
        lang, buffer.get(), str.size(), cargs, session->getFileSystem(),
        options, CancellationToken,
        [&](CancellableResult<CodeCompleteResult> Result) {
          CallbackCalled = true;
          switch (Result.getKind()) {
          case CancellableResultKind::Success: {
            auto *context = Result->Info.completionContext;
            if (!context ||
                context->CodeCompletionKind != CompletionKind::PostfixExpr) {
              return;
            }
            auto topResults = filterInnerResults(
                ArrayRef<CodeCompletionResult *>(Result->ResultSink.Results),
                options.addInnerResults, options.addInnerOperators, hasDot,
                hasQDot, hasInit, rules);
            // FIXME: Clearing the flair (and semantic context) is a hack so
            // that they won't overwhelm other results that also match the
            // filter text.
            innerResults =
                extendCompletions(topResults, innerSink, Result->Info,
                                  nameToPopularity, options, exactMatch,
                                  /*clearFlair=*/true);
            innerSink.adoptCodiraSink(Result->ResultSink);
            break;
          }
          case CancellableResultKind::Failure:
            CodeCompleteDidFail = true;
            consumer.failed(Result.getError());
            break;
          case CancellableResultKind::Cancelled:
            CodeCompleteDidFail = true;
            consumer.cancelled();
            break;
          }
        });
    assert(CallbackCalled &&
           "Expected the callback to be called synchronously");

    if (CodeCompleteDidFail) {
      // We already informed the consumer that completion failed. No need to
      // continue.
      return;
    }

    if (options.addInnerOperators) {
      if (hasInit && !rules.hideFilterName("("))
        innerResults.insert(innerResults.begin(), buildParen());
      if (hasDot && !rules.hideFilterName("."))
        innerResults.insert(innerResults.begin(), buildDot());
      if (hasQDot && !rules.hideFilterName("?."))
        innerResults.insert(innerResults.begin(), buildQDot());
    }

    // Add the inner results (and don't filter them).
    exactMatch = nullptr; // No longer needed.
    organizer.addCompletionsWithFilter(innerResults, filterText,
                                       CodeCompletion::FilterRules(), exactMatch);

    CodeCompletion::Options noGroupOpts = options;
    noGroupOpts.groupStems = false;
    noGroupOpts.groupOverloads = false;
    organizer.groupAndSort(noGroupOpts);
  }

  // Build the final results view.
  auto view = organizer.takeResultsView();
  CodeCompletion::LimitedResultView limitedResults(*view, resultOffset,
                                                   maxResults);

  // Forward results to the SourceKit consumer.
  CodiraGroupedCodeCompletionConsumer groupedConsumer(consumer);
  limitedResults.walk(groupedConsumer);
  consumer.setNextRequestStart(limitedResults.getNextOffset());
}

void CodiraLangSupport::codeCompleteOpen(
    StringRef name, toolchain::MemoryBuffer *inputBuf, unsigned offset,
    OptionsDictionary *options, ArrayRef<FilterRule> rawFilterRules,
    GroupedCodeCompletionConsumer &consumer, ArrayRef<const char *> args,
    std::optional<VFSOptions> vfsOptions,
    SourceKitCancellationToken CancellationToken) {
  StringRef filterText;
  unsigned resultOffset = 0;
  unsigned maxResults = 0;
  CodeCompletion::Options CCOpts;
  // Enable "call pattern heuristics" by default for this API.
  if (options)
    translateCodeCompletionOptions(*options, CCOpts, filterText, resultOffset,
                                   maxResults);

  std::string fileSystemError;
  // FIXME: the use of None as primary file is to match the fact we do not read
  // the document contents using the editor documents infrastructure.
  auto fileSystem =
      getFileSystem(vfsOptions, /*primaryFile=*/std::nullopt, fileSystemError);
  if (!fileSystem)
    return consumer.failed(fileSystemError);

  CodeCompletion::FilterRules filterRules;
  translateFilterRules(rawFilterRules, filterRules);

  // Set up the code completion consumer to pass results to organizer.
  CodeCompletion::CompletionSink sink;
  std::vector<Completion *> completions;

  NameToPopularityMap *nameToPopularity = nullptr;
  // This reference must outlive the uses of nameToPopularity.
  auto popularAPIRef = PopularAPI;
  if (popularAPIRef) {
    nameToPopularity = &popularAPIRef->nameToFactor;
  }

  CompletionKind completionKind = CompletionKind::None;
  TypeContextKind typeContextKind = TypeContextKind::None;
  bool mayUseImplicitMemberExpr = false;

  bool CodeCompleteDidFail = false;
  bool CallbackCalled = false;
  languageCodeCompleteImpl(
      *this, inputBuf, offset, args, fileSystem, CCOpts, CancellationToken,
      [&](CancellableResult<CodeCompleteResult> Result) {
        CallbackCalled = true;
        switch (Result.getKind()) {
        case CancellableResultKind::Success: {
          auto &completionCtx = *Result->Info.completionContext;
          completionKind = completionCtx.CodeCompletionKind;
          typeContextKind = completionCtx.typeContextKind;
          mayUseImplicitMemberExpr = completionCtx.MayUseImplicitMemberExpr;
          consumer.setReusingASTContext(completionCtx.ReusingASTContext);
          consumer.setAnnotatedTypename(completionCtx.getAnnotateResult());
          completions =
              extendCompletions(Result->ResultSink.Results, sink, Result->Info,
                                nameToPopularity, CCOpts);
          sink.adoptCodiraSink(Result->ResultSink);
          break;
        }
        case CancellableResultKind::Failure:
          CodeCompleteDidFail = true;
          consumer.failed(Result.getError());
          break;
        case CancellableResultKind::Cancelled:
          CodeCompleteDidFail = true;
          consumer.cancelled();
          break;
        }
      });
  assert(CallbackCalled && "Expected the callback to be called synchronously");

  if (CodeCompleteDidFail) {
    // We already informed the consumer that completion failed. No need to
    // continue.
    return;
  }

  // Add any relevant custom completions.
  if (auto custom = CustomCompletions)
    addCustomCompletions(sink, completions, custom->customCompletions,
                         completionKind);

  // Pre-sort the completions.
  CodeCompletion::CodeCompletionOrganizer::preSortCompletions(completions);

  // Store in the session.
  using CodeCompletion::SessionCache;
  using CodeCompletion::SessionCacheRef;
  auto bufferCopy = toolchain::MemoryBuffer::getMemBufferCopy(
      inputBuf->getBuffer(), inputBuf->getBufferIdentifier());
  std::vector<std::string> argsCopy(args.begin(), args.end());
  SessionCacheRef session{new SessionCache(
      std::move(sink), std::move(bufferCopy), std::move(argsCopy), fileSystem,
      completionKind, typeContextKind, mayUseImplicitMemberExpr,
      std::move(filterRules))};
  session->setSortedCompletions(std::move(completions));

  if (!CCSessions.set(name, offset, session)) {
    std::string err;
    toolchain::raw_string_ostream OS(err);
    OS << "codecomplete.open: code completion session for '" << name << "', "
       << offset << " already exists";
    consumer.failed(OS.str());
  }

  transformAndForwardResults(consumer, *this, session, nameToPopularity, CCOpts,
                             offset, filterText, resultOffset, maxResults,
                             CancellationToken);
}

void CodiraLangSupport::codeCompleteClose(
    StringRef name, unsigned offset, GroupedCodeCompletionConsumer &consumer) {
  if (!CCSessions.remove(name, offset)) {
    std::string err;
    toolchain::raw_string_ostream OS(err);
    OS << "codecomplete.close: no code completion session for '" << name
       << "', " << offset;
    consumer.failed(OS.str());
  }
}

void CodiraLangSupport::codeCompleteUpdate(
    StringRef name, unsigned offset, OptionsDictionary *options,
    SourceKitCancellationToken CancellationToken,
    GroupedCodeCompletionConsumer &consumer) {
  auto session = CCSessions.get(name, offset);
  if (!session) {
    std::string err;
    toolchain::raw_string_ostream OS(err);
    OS << "codecomplete.update: no code completion session for '" << name
       << "', " << offset;
    consumer.failed(OS.str());
    return;
  }

  StringRef filterText;
  unsigned resultOffset = 0;
  unsigned maxResults = 0;

  // FIXME: consider whether CCOpts has changed since the 'open'.
  CodeCompletion::Options CCOpts;
  if (options)
    translateCodeCompletionOptions(*options, CCOpts, filterText, resultOffset,
                                   maxResults);

  NameToPopularityMap *nameToPopularity = nullptr;
  // This reference must outlive the uses of nameToPopularity.
  auto popularAPIRef = PopularAPI;
  if (popularAPIRef) {
    nameToPopularity = &popularAPIRef->nameToFactor;
  }

  transformAndForwardResults(consumer, *this, session, nameToPopularity, CCOpts,
                             offset, filterText, resultOffset, maxResults,
                             CancellationToken);
}

language::ide::CodeCompletionCache &CodiraCompletionCache::getCache() {
  return *inMemory;
}
CodiraCompletionCache::~CodiraCompletionCache() {}

void CodiraLangSupport::codeCompleteCacheOnDisk(StringRef path) {
  ThreadSafeRefCntPtr<CodiraCompletionCache> newCache(new CodiraCompletionCache);
  newCache->onDisk = std::make_unique<ide::OnDiskCodeCompletionCache>(path);
  newCache->inMemory =
      std::make_unique<ide::CodeCompletionCache>(newCache->onDisk.get());

  CCCache = newCache; // replace the old cache.
}

void CodiraLangSupport::codeCompleteSetPopularAPI(
    ArrayRef<const char *> popularAPI, ArrayRef<const char *> unpopularAPI) {
  using Factor = CodeCompletion::PopularityFactor;
  ThreadSafeRefCntPtr<CodiraPopularAPI> newPopularAPI(new CodiraPopularAPI);
  auto &nameToFactor = newPopularAPI->nameToFactor;
  for (unsigned i = 0, n = popularAPI.size(); i < n; ++i) {
    SmallString<64> name;
    if (canonicalizeFilterName(popularAPI[i], name))
      continue;
    nameToFactor[name] = Factor(double(n - i) / n);
  }
  for (unsigned i = 0, n = unpopularAPI.size(); i < n; ++i) {
    SmallString<64> name;
    if (canonicalizeFilterName(unpopularAPI[i], name))
      continue;
    nameToFactor[name] = Factor(-double(n - i) / n);
  }

  PopularAPI = newPopularAPI; // replace the old popular API.
}

void CodiraLangSupport::codeCompleteSetCustom(
    ArrayRef<CustomCompletionInfo> completions) {
  ThreadSafeRefCntPtr<CodiraCustomCompletions> newCustomCompletions(
      new CodiraCustomCompletions);
  newCustomCompletions->customCompletions.assign(completions.begin(),
                                                 completions.end());
  CustomCompletions = newCustomCompletions; // Replace the existing completions.
}
