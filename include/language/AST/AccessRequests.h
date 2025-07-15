//===--- AccessRequests.h - Access{Level,Scope} Requests -----*- C++ -*-===//
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
//  This file defines access-control requests.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_ACCESS_REQUESTS_H
#define LANGUAGE_ACCESS_REQUESTS_H

#include "language/AST/AccessScope.h"
#include "language/AST/AttrKind.h"
#include "language/AST/Evaluator.h"
#include "language/AST/SimpleRequest.h"
#include "language/Basic/Statistic.h"
#include "toolchain/ADT/Hashing.h"

namespace language {

class AbstractStorageDecl;
class ExtensionDecl;
class ValueDecl;

/// Request the AccessLevel of the given ValueDecl.
class AccessLevelRequest :
    public SimpleRequest<AccessLevelRequest,
                         AccessLevel(ValueDecl *),
                         RequestFlags::SeparatelyCached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  AccessLevel evaluate(Evaluator &evaluator, ValueDecl *decl) const;

public:
  // Separate caching.
  bool isCached() const { return true; }
  std::optional<AccessLevel> getCachedResult() const;
  void cacheResult(AccessLevel value) const;
};

/// Request the setter AccessLevel of the given AbstractStorageDecl,
/// which may be lower than its normal AccessLevel, and determines
/// the accessibility of mutating accessors.
class SetterAccessLevelRequest :
    public SimpleRequest<SetterAccessLevelRequest,
                         AccessLevel(AbstractStorageDecl *),
                         RequestFlags::SeparatelyCached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  AccessLevel evaluate(Evaluator &evaluator, AbstractStorageDecl *decl) const;

public:
  // Separate caching.
  bool isCached() const { return true; }
  std::optional<AccessLevel> getCachedResult() const;
  void cacheResult(AccessLevel value) const;
};

using DefaultAndMax = std::pair<AccessLevel, AccessLevel>;

/// Request the Default and Max AccessLevels of the given ExtensionDecl.
class DefaultAndMaxAccessLevelRequest :
    public SimpleRequest<DefaultAndMaxAccessLevelRequest,
                         DefaultAndMax(ExtensionDecl *),
                         RequestFlags::SeparatelyCached> {
public:
  using SimpleRequest::SimpleRequest;
private:
  friend SimpleRequest;

  // Evaluation.
  DefaultAndMax evaluate(Evaluator &evaluator, ExtensionDecl *decl) const;

public:
  // Separate caching.
  bool isCached() const { return true; }
  std::optional<DefaultAndMax> getCachedResult() const;
  void cacheResult(DefaultAndMax value) const;
};

#define LANGUAGE_TYPEID_ZONE AccessControl
#define LANGUAGE_TYPEID_HEADER "language/AST/AccessTypeIDZone.def"
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
#include "language/AST/AccessTypeIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_ACCESS_REQUESTS_H
