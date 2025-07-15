//===--- ParseRequests.h - Parsing Requests ---------------------*- C++ -*-===//
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
//
//  This file defines parsing requests.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_PARSE_REQUESTS_H
#define LANGUAGE_PARSE_REQUESTS_H

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/EvaluatorDependencies.h"
#include "language/AST/SimpleRequest.h"
#include "language/Basic/Fingerprint.h"
#include "language/Parse/Token.h"

namespace language {

struct ASTNode;
class AvailabilityMacroMap;

/// Report that a request of the given kind is being evaluated, so it
/// can be recorded by the stats reporter.
template<typename Request>
void reportEvaluatedRequest(UnifiedStatsReporter &stats,
                            const Request &request);

struct FingerprintAndMembers {
  std::optional<Fingerprint> fingerprint = std::nullopt;
  ArrayRef<Decl *> members = {};
  bool operator==(const FingerprintAndMembers &x) const {
    return fingerprint == x.fingerprint && members == x.members;
  }
};

void simple_display(toolchain::raw_ostream &out, const FingerprintAndMembers &value);

/// Parse the members of a nominal type declaration or extension.
/// Return a fingerprint and the members.
class ParseMembersRequest
    : public SimpleRequest<ParseMembersRequest,
                           FingerprintAndMembers(IterableDeclContext *),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  FingerprintAndMembers evaluate(Evaluator &evaluator,
                                 IterableDeclContext *idc) const;

public:
  // Caching
  bool isCached() const { return true; }
};

/// Parse the body of a function, initializer, or deinitializer.
class ParseAbstractFunctionBodyRequest
    : public SimpleRequest<ParseAbstractFunctionBodyRequest,
                           BodyAndFingerprint(AbstractFunctionDecl *),
                           RequestFlags::SeparatelyCached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  BodyAndFingerprint evaluate(Evaluator &evaluator,
                              AbstractFunctionDecl *afd) const;

public:
  // Caching
  bool isCached() const { return true; }
  std::optional<BodyAndFingerprint> getCachedResult() const;
  void cacheResult(BodyAndFingerprint value) const;
};

struct SourceFileParsingResult {
  ArrayRef<ASTNode> TopLevelItems;
  std::optional<ArrayRef<Token>> CollectedTokens;
  std::optional<Fingerprint> Fingerprint;
};

/// Parse the top-level items of a SourceFile.
class ParseSourceFileRequest
    : public SimpleRequest<
          ParseSourceFileRequest, SourceFileParsingResult(SourceFile *),
          RequestFlags::SeparatelyCached | RequestFlags::DependencySource> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  SourceFileParsingResult evaluate(Evaluator &evaluator, SourceFile *SF) const;

public:
  // Caching.
  bool isCached() const { return true; }
  std::optional<SourceFileParsingResult> getCachedResult() const;
  void cacheResult(SourceFileParsingResult result) const;

public:
  evaluator::DependencySource
  readDependencySource(const evaluator::DependencyRecorder &) const;
};

/// Parse the ExportedSourceFile for a given SourceFile.
class ExportedSourceFileRequest
    : public SimpleRequest<ExportedSourceFileRequest,
                           void *(const SourceFile *),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  void *evaluate(Evaluator &evaluator, const SourceFile *SF) const;

public:
  // Cached.
  bool isCached() const { return true; }
};

/// Parse the top-level items of a SourceFile.
class ParseTopLevelDeclsRequest
    : public SimpleRequest<
          ParseTopLevelDeclsRequest, ArrayRef<Decl *>(SourceFile *),
          RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ArrayRef<Decl *> evaluate(Evaluator &evaluator, SourceFile *SF) const;

public:
  // Caching.
  bool isCached() const { return true; }
};

void simple_display(toolchain::raw_ostream &out,
                    const IDEInspectionCallbacksFactory *factory);

class IDEInspectionSecondPassRequest
    : public SimpleRequest<IDEInspectionSecondPassRequest,
                           bool(SourceFile *, IDEInspectionCallbacksFactory *),
                           RequestFlags::Uncached|RequestFlags::DependencySource> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  bool evaluate(Evaluator &evaluator, SourceFile *SF,
                IDEInspectionCallbacksFactory *Factory) const;

public:
  evaluator::DependencySource
  readDependencySource(const evaluator::DependencyRecorder &) const;
};

class EvaluateIfConditionRequest
    : public SimpleRequest<EvaluateIfConditionRequest,
          std::pair<bool, bool>(SourceFile *, SourceRange conditionRange,
                                bool shouldEvaluate),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  std::pair<bool, bool> evaluate(
      Evaluator &evaluator, SourceFile *SF, SourceRange conditionRange,
      bool shouldEvaluate) const;
};

/// Parse the '-define-availability' arguments.
class AvailabilityMacroArgumentsRequest
    : public SimpleRequest<AvailabilityMacroArgumentsRequest,
                           const AvailabilityMacroMap *(ASTContext *),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  const AvailabilityMacroMap *evaluate(Evaluator &evaluator,
                                       ASTContext *ctx) const;

public:
  // Caching.
  bool isCached() const { return true; }

  // Source location.
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

void simple_display(toolchain::raw_ostream &out, const ASTContext *state);

/// The zone number for the parser.
#define LANGUAGE_TYPEID_ZONE Parse
#define LANGUAGE_TYPEID_HEADER "language/AST/ParseTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER

// Set up reporting of evaluated requests.
#define LANGUAGE_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
  template <>                                                                  \
  inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,              \
                                     const RequestType &request) {             \
    ++stats.getFrontendCounters().RequestType;                                 \
  }
#include "language/AST/ParseTypeIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_PARSE_REQUESTS_H
