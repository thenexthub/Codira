//===------- ConstExtractRequests.h - Extraction  Requests ------*- C++ -*-===//
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
//  This file defines const-extraction requests.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CONST_EXTRACT_REQUESTS_H
#define LANGUAGE_CONST_EXTRACT_REQUESTS_H

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/ConstTypeInfo.h"
#include "language/AST/EvaluatorDependencies.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Identifier.h"
#include "language/AST/NameLookup.h"
#include "language/AST/SimpleRequest.h"
#include "language/Basic/Statistic.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/TinyPtrVector.h"

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
              toolchain::PointerUnion<const SourceFile *, ModuleDecl *>),
          RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ConstValueTypeInfo
  evaluate(Evaluator &eval, NominalTypeDecl *nominal,
           toolchain::PointerUnion<const SourceFile *, ModuleDecl *> extractionScope)
      const;

public:
  // Caching
  bool isCached() const { return true; }
};

#define LANGUAGE_TYPEID_ZONE ConstExtract
#define LANGUAGE_TYPEID_HEADER "language/ConstExtract/ConstExtractTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER

// Set up reporting of evaluated requests.
template<typename Request>
void reportEvaluatedRequest(UnifiedStatsReporter &stats,
                            const Request &request);

#define LANGUAGE_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
  template <>                                                                  \
  inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,              \
                                     const RequestType &request) {             \
    ++stats.getFrontendCounters().RequestType;                                 \
  }
#include "language/ConstExtract/ConstExtractTypeIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_CONST_EXTRACT_REQUESTS_H

