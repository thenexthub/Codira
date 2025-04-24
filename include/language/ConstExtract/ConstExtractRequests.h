//===------- ConstExtractRequests.h - Extraction  Requests ------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2021 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file defines const-extraction requests.
//
//===----------------------------------------------------------------------===//
#ifndef SWIFT_CONST_EXTRACT_REQUESTS_H
#define SWIFT_CONST_EXTRACT_REQUESTS_H

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/ConstTypeInfo.h"
#include "language/AST/EvaluatorDependencies.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Identifier.h"
#include "language/AST/NameLookup.h"
#include "language/AST/SimpleRequest.h"
#include "language/Basic/Statistic.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/TinyPtrVector.h"

namespace language {

class Decl;
class DeclName;
class EnumDecl;

/// Retrieve information about compile-time-known values
class ConstantValueInfoRequest
    : public SimpleRequest<
          ConstantValueInfoRequest,
          ConstValueTypeInfo(
              NominalTypeDecl *,
              llvm::PointerUnion<const SourceFile *, ModuleDecl *>),
          RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ConstValueTypeInfo
  evaluate(Evaluator &eval, NominalTypeDecl *nominal,
           llvm::PointerUnion<const SourceFile *, ModuleDecl *> extractionScope)
      const;

public:
  // Caching
  bool isCached() const { return true; }
};

#define SWIFT_TYPEID_ZONE ConstExtract
#define SWIFT_TYPEID_HEADER "swift/ConstExtract/ConstExtractTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"
#undef SWIFT_TYPEID_ZONE
#undef SWIFT_TYPEID_HEADER

// Set up reporting of evaluated requests.
template<typename Request>
void reportEvaluatedRequest(UnifiedStatsReporter &stats,
                            const Request &request);

#define SWIFT_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
  template <>                                                                  \
  inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,              \
                                     const RequestType &request) {             \
    ++stats.getFrontendCounters().RequestType;                                 \
  }
#include "language/ConstExtract/ConstExtractTypeIDZone.def"
#undef SWIFT_REQUEST

} // end namespace language

#endif // SWIFT_CONST_EXTRACT_REQUESTS_H

