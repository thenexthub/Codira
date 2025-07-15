//===--- CodeCompletion.h - -------------------------------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_CODECOMPLETION_H
#define TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_CODECOMPLETION_H

#include "SourceKit/Core/Toolchain.h"
#include "language/Basic/StringExtras.h"
#include "language/IDE/CodeCompletionResult.h"
#include "language/IDE/CodeCompletionResultSink.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringMap.h"
#include <optional>

namespace SourceKit {
namespace CodeCompletion {

using language::NullTerminatedStringRef;
using language::ide::CodeCompletionDeclKind;
using language::ide::CodeCompletionFlair;
using language::ide::CodeCompletionKeywordKind;
using language::ide::CodeCompletionLiteralKind;
using language::ide::CodeCompletionOperatorKind;
using language::ide::CodeCompletionResultKind;
using language::ide::CodeCompletionResultTypeRelation;
using language::ide::CodeCompletionString;
using language::ide::SemanticContextKind;
using CodiraResult = language::ide::CodeCompletionResult;
using language::ide::CompletionKind;

struct Group;
class CodeCompletionOrganizer;
class CompletionBuilder;

/// A representation of the 'popularity' of a code completion result.
struct PopularityFactor {
  /// Raw popularity score in the range [-1, 1], where higher values are more
  /// popular and 0.0 indicates an unknown popularity.
  double rawValue = 0.0;
  bool isPopular() const { return rawValue > 0.0; }
  bool isUnpopular() const { return rawValue < 0.0; }
  PopularityFactor() = default;
  explicit PopularityFactor(double value) : rawValue(value) {}
};

struct NameStyle {
  enum WordDelimiter : uint8_t {
    Unknown,
    Lowercase,                // lowercase
    Uppercase,                // UPPERCASE
    UpperCamelCase,           // UpperCamelCase
    LowerCamelCase,           // lowerCamelCase
    LowercaseWithUnderscores, // lowercase_with_underscores
    UppercaseWithUnderscores, // UPPERCASE_WITH_UNDERSCORES
  };

  WordDelimiter wordDelimiter = Unknown;
  uint8_t leadingUnderscores : 2;
  uint8_t trailingUnderscores : 2;

  explicit NameStyle(StringRef name);

  bool possiblyLowerCamelCase() const {
    return wordDelimiter == Lowercase || wordDelimiter == LowerCamelCase;
  }
  bool possiblyUpperCamelCase() const {
    return wordDelimiter == Uppercase || wordDelimiter == UpperCamelCase;
  }
};

/// Code completion result type for SourceKit::CodiraLangSupport.
///
/// Extends a \c language::ide::CodeCompletionResult with extra fields that are
/// filled in by SourceKit. Generally stored in an \c CompletionSink.
class Completion {
  const CodiraResult &base;
  void *opaqueCustomKind = nullptr;
  std::optional<uint8_t> moduleImportDepth;
  PopularityFactor popularityFactor;
  StringRef name;
  StringRef description;
  friend class CompletionBuilder;

public:
  static constexpr unsigned numSemanticContexts = 8;
  static constexpr unsigned maxModuleImportDepth = 10;

  /// Wraps \p base with an \c Completion.  The \p name and \p description
  /// should outlive the result, generally by being stored in the same
  /// \c CompletionSink or in a sink that was adopted by the sink that this
  /// \c Compleiton is being stored in.
  Completion(const CodiraResult &base, StringRef description)
      : base(base), description(description) {}

  const CodiraResult &getCodiraResult() const { return base; }

  bool hasCustomKind() const { return opaqueCustomKind; }
  void *getCustomKind() const { return opaqueCustomKind; }
  StringRef getDescription() const { return description; }
  std::optional<uint8_t> getModuleImportDepth() const {
    return moduleImportDepth;
  }

  /// A popularity factory in the range [-1, 1]. The higher the value, the more
  /// 'popular' this result is.  0 indicates unknown.
  PopularityFactor getPopularityFactor() const { return popularityFactor; }

  // MARK: Methods that forward to the CodiraResult

  CodeCompletionResultKind getKind() const {
    return getCodiraResult().getKind();
  }

  CodeCompletionDeclKind getAssociatedDeclKind() const {
    return getCodiraResult().getAssociatedDeclKind();
  }

  CodeCompletionLiteralKind getLiteralKind() const {
    return getCodiraResult().getLiteralKind();
  }

  CodeCompletionKeywordKind getKeywordKind() const {
    return getCodiraResult().getKeywordKind();
  }

  bool isOperator() const { return getCodiraResult().isOperator(); }

  CodeCompletionOperatorKind getOperatorKind() const {
    return getCodiraResult().getOperatorKind();
  }

  bool isSystem() const { return getCodiraResult().isSystem(); }

  CodeCompletionResultTypeRelation getExpectedTypeRelation() const {
    return getCodiraResult().getExpectedTypeRelation();
  }

  SemanticContextKind getSemanticContext() const {
    return getCodiraResult().getSemanticContext();
  }

  CodeCompletionFlair getFlair() const { return getCodiraResult().getFlair(); }

  bool isNotRecommended() const { return getCodiraResult().isNotRecommended(); }

  unsigned getNumBytesToErase() const {
    return getCodiraResult().getNumBytesToErase();
  }

  CodeCompletionString *getCompletionString() const {
    return getCodiraResult().getCompletionString();
  }

  StringRef getModuleName() const { return getCodiraResult().getModuleName(); }

  StringRef getBriefDocComment() const {
    return getCodiraResult().getBriefDocComment();
  }

  ArrayRef<NullTerminatedStringRef> getAssociatedUSRs() const {
    return getCodiraResult().getAssociatedUSRs();
  }

  StringRef getFilterName() const {
    return getCodiraResult().getFilterName();
  }

  /// Allow "upcasting" the completion result to a CodiraResult.
  operator const CodiraResult &() const { return getCodiraResult(); }
};

/// Storage sink for \c Completion objects.
///
/// In addition to allocating the results themselves, uses \c languageSink to keep
/// the storage for the underlying language results alive.
struct CompletionSink {
  language::ide::CodeCompletionResultSink languageSink;
  toolchain::BumpPtrAllocator allocator;

  /// Adds references to a language sink's allocators to keep its storage alive.
  void adoptCodiraSink(language::ide::CodeCompletionResultSink &sink) {
    languageSink.ForeignAllocators.insert(languageSink.ForeignAllocators.end(),
                                       sink.ForeignAllocators.begin(),
                                       sink.ForeignAllocators.end());
    languageSink.ForeignAllocators.push_back(sink.Allocator);
  }
};

class CompletionBuilder {
  CompletionSink &sink;
  const CodiraResult &base;
  bool modified = false;
  SemanticContextKind semanticContext;
  CodeCompletionFlair flair;
  CodeCompletionString *completionString;
  void *customKind = nullptr;
  std::optional<uint8_t> moduleImportDepth;
  PopularityFactor popularityFactor;

public:
  CompletionBuilder(CompletionSink &sink, const CodiraResult &base);

  void setCustomKind(void *opaqueCustomKind) { customKind = opaqueCustomKind; }

  void setModuleImportDepth(std::optional<uint8_t> value) {
    assert(!value || *value <= Completion::maxModuleImportDepth);
    moduleImportDepth = value;
  }

  void setSemanticContext(SemanticContextKind kind) {
    modified = true;
    semanticContext = kind;
  }
  void setFlair(CodeCompletionFlair value) {
    modified = true;
    flair = value;
  }

  void setPopularityFactor(PopularityFactor val) { popularityFactor = val; }

  void setPrefix(CodeCompletionString *prefix);

  StringRef getOriginalName() const {
    return base.getFilterName();
  }

  Completion *finish();
};


/// Immutable view of code completion results.
///
/// Provides a possibly filtered view of code completion results
/// (\c Completion) organized into groups.  Clients walk the tree using
/// CodeCompletionView::Walker.  The \c Completion objects are not owned
/// by the view and must outlive it.
class CodeCompletionView {
  const Group *rootGroup = nullptr; ///< Owned by the view.

  friend class CodeCompletionOrganizer;
  friend class LimitedResultView;
  CodeCompletionView(const CodeCompletionView &) = delete;
  void operator=(const CodeCompletionView &) = delete;
public:

  CodeCompletionView() = default;
  CodeCompletionView(CodeCompletionView &&) = default;
  virtual ~CodeCompletionView();

  struct Walker;
  virtual bool walk(Walker &walker) const;
};

/// Interface implemented by clients of \c CodeCompletionView.
struct CodeCompletionView::Walker {
  virtual ~Walker() {}
  virtual bool handleResult(Completion *result) = 0;
  virtual void startGroup(StringRef name) = 0;
  virtual void endGroup() = 0;
};

using CodeCompletionViewRef = std::shared_ptr<const CodeCompletionView>;

class LimitedResultView : public CodeCompletionView {
  const CodeCompletionView &baseView;
  mutable unsigned start;
  unsigned maxResults;

public:
  LimitedResultView(const CodeCompletionView &baseView, unsigned start,
                    unsigned maxResults)
      : baseView(baseView), start(start), maxResults(maxResults) {}

  unsigned getNextOffset() const;
  bool walk(Walker &walker) const override;
};

struct FilterRules {
  bool hideAll = false;

  bool hideAllValueLiterals = false;
  toolchain::SmallDenseMap<CodeCompletionLiteralKind, bool, 8> hideValueLiteral;

  bool hideAllKeywords = false;
  toolchain::DenseMap<CodeCompletionKeywordKind, bool> hideKeyword;

  bool hideCustomCompletions = false;
  // FIXME: hide individual custom completions

  toolchain::StringMap<bool> hideModule;
  toolchain::StringMap<bool> hideByFilterName;
  toolchain::StringMap<bool> hideByDescription;

  bool hideCompletion(const Completion &completion) const;
  bool hideCompletion(const CodiraResult &completion, StringRef name,
                      StringRef description, void *customKind = nullptr) const;
  bool hideFilterName(StringRef name) const;
};

} // end namespace CodeCompletion
} // end namespace SourceKit

#endif // TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_CODECOMPLETION_H
