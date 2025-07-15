//===----- IDERequests.h - IDE functionality Requests -----------*- C++ -*-===//
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
//  This file defines IDE request using the evaluator model.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_IDE_REQUESTS_H
#define LANGUAGE_IDE_REQUESTS_H

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/Evaluator.h"
#include "language/AST/SimpleRequest.h"
#include "language/AST/SourceFile.h"
#include "language/IDE/Utils.h"
#include "language/IDE/IDETypeIDs.h"

namespace language {
//----------------------------------------------------------------------------//
// Cusor info
//----------------------------------------------------------------------------//

// Input for CursorInfoRequest.
// Putting the source file and location together allows us to print the request
// input well e.g. file.code:3:4
struct CursorInfoOwner {
  SourceFile *File;
  SourceLoc Loc;

  CursorInfoOwner(SourceFile *File, SourceLoc Loc): File(File), Loc(Loc) { }

  friend toolchain::hash_code hash_value(const CursorInfoOwner &CI) {
    return toolchain::hash_combine(CI.File, CI.Loc.getOpaquePointerValue());
  }

  friend bool operator==(const CursorInfoOwner &lhs, const CursorInfoOwner &rhs) {
    return lhs.File == rhs.File &&
      lhs.Loc.getOpaquePointerValue() == rhs.Loc.getOpaquePointerValue();
  }

  friend bool operator!=(const CursorInfoOwner &lhs, const CursorInfoOwner &rhs) {
    return !(lhs == rhs);
  }
  bool isValid() const {
    return File && Loc.isValid();
  }
};

void simple_display(toolchain::raw_ostream &out, const CursorInfoOwner &owner);

/// Resolve cursor info at a given location.
class CursorInfoRequest
    : public SimpleRequest<CursorInfoRequest,
                           ide::ResolvedCursorInfoPtr(CursorInfoOwner),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ide::ResolvedCursorInfoPtr evaluate(Evaluator &evaluator,
                                      CursorInfoOwner CI) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const;
};

//----------------------------------------------------------------------------//
// Range info
//----------------------------------------------------------------------------//

// Input for RangeInfoRequest.
// Putting the source file and location together allows us to print the request
// input well e.g. file.code:3:4
struct RangeInfoOwner {
  SourceFile *File;
  SourceLoc StartLoc;
  SourceLoc EndLoc;

  RangeInfoOwner(SourceFile *File, SourceLoc StartLoc, SourceLoc EndLoc):
    File(File), StartLoc(StartLoc), EndLoc(EndLoc) {}
  RangeInfoOwner(SourceFile *File, unsigned Offset, unsigned Length);

  friend toolchain::hash_code hash_value(const RangeInfoOwner &CI) {
    return toolchain::hash_combine(CI.File,
                              CI.StartLoc.getOpaquePointerValue(),
                              CI.EndLoc.getOpaquePointerValue());
  }

  friend bool operator==(const RangeInfoOwner &lhs, const RangeInfoOwner &rhs) {
    return lhs.File == rhs.File && lhs.StartLoc == rhs.StartLoc &&
      lhs.EndLoc == rhs.EndLoc;
  }

  friend bool operator!=(const RangeInfoOwner &lhs, const RangeInfoOwner &rhs) {
    return !(lhs == rhs);
  }

  bool isValid() const {
    return File && StartLoc.isValid() && EndLoc.isValid();
  }
};

void simple_display(toolchain::raw_ostream &out, const RangeInfoOwner &owner);

/// Resolve cursor info at a given location.
class RangeInfoRequest:
    public SimpleRequest<RangeInfoRequest,
                         ide::ResolvedRangeInfo(RangeInfoOwner),
                         RequestFlags::Cached>
{
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ide::ResolvedRangeInfo evaluate(Evaluator &evaluator,
                                  RangeInfoOwner CI) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const;
};

//----------------------------------------------------------------------------//
// ProvideDefaultImplForRequest
//----------------------------------------------------------------------------//
/// Collect all the protocol requirements that a given declaration can
///   provide default implementations for. Input is a declaration in extension
///   declaration. The result is an array of requirements.
class ProvideDefaultImplForRequest:
    public SimpleRequest<ProvideDefaultImplForRequest,
                         ArrayRef<ValueDecl*>(ValueDecl*),
                         RequestFlags::Cached>
{
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ArrayRef<ValueDecl*> evaluate(Evaluator &evaluator,
                                ValueDecl* VD) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

//----------------------------------------------------------------------------//
// CollectOverriddenDeclsRequest
//----------------------------------------------------------------------------//
struct OverridenDeclsOwner {
  ValueDecl *VD;
  bool IncludeProtocolRequirements;
  bool Transitive;

  OverridenDeclsOwner(ValueDecl *VD, bool IncludeProtocolRequirements,
    bool Transitive): VD(VD),
      IncludeProtocolRequirements(IncludeProtocolRequirements),
      Transitive(Transitive) {}

  friend toolchain::hash_code hash_value(const OverridenDeclsOwner &CI) {
    return toolchain::hash_combine(CI.VD,
                              CI.IncludeProtocolRequirements,
                              CI.Transitive);
  }

  friend bool operator==(const OverridenDeclsOwner &lhs,
                         const OverridenDeclsOwner &rhs) {
    return lhs.VD == rhs.VD &&
      lhs.IncludeProtocolRequirements == rhs.IncludeProtocolRequirements &&
      lhs.Transitive == rhs.Transitive;
  }

  friend bool operator!=(const OverridenDeclsOwner &lhs,
                         const OverridenDeclsOwner &rhs) {
    return !(lhs == rhs);
  }

  friend void simple_display(toolchain::raw_ostream &out,
                             const OverridenDeclsOwner &owner) {
    simple_display(out, owner.VD);
  }
};

/// Get decls that the given decl overrides, protocol requirements that
///   it serves as a default implementation of, and optionally protocol
///   requirements it satisfies in a conforming class
class CollectOverriddenDeclsRequest:
    public SimpleRequest<CollectOverriddenDeclsRequest,
                         ArrayRef<ValueDecl*>(OverridenDeclsOwner),
                         RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ArrayRef<ValueDecl*> evaluate(Evaluator &evaluator,
                                OverridenDeclsOwner Owner) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

//----------------------------------------------------------------------------//
// ResolveProtocolNameRequest
//----------------------------------------------------------------------------//
struct ProtocolNameOwner {
  DeclContext *DC;
  std::string Name;
  ProtocolNameOwner(DeclContext *DC, StringRef Name): DC(DC), Name(Name) {}

  friend toolchain::hash_code hash_value(const ProtocolNameOwner &CI) {
    return hash_value(CI.Name);
  }

  friend bool operator==(const ProtocolNameOwner &lhs,
                         const ProtocolNameOwner &rhs) {
    return lhs.Name == rhs.Name;
  }

  friend bool operator!=(const ProtocolNameOwner &lhs,
                         const ProtocolNameOwner &rhs) {
    return !(lhs == rhs);
  }

  friend void simple_display(toolchain::raw_ostream &out,
                             const ProtocolNameOwner &owner) {
    out << "Resolve " << owner.Name << " from ";
    simple_display(out, owner.DC);
  }
};

/// Resolve a protocol name to the protocol decl pointer inside the ASTContext
class ResolveProtocolNameRequest:
    public SimpleRequest<ResolveProtocolNameRequest,
                         ProtocolDecl*(ProtocolNameOwner),
                         RequestFlags::Cached>
{
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ProtocolDecl *evaluate(Evaluator &evaluator,
                         ProtocolNameOwner Input) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

/// The zone number for the IDE.
#define LANGUAGE_TYPEID_ZONE IDE
#define LANGUAGE_TYPEID_HEADER "language/IDE/IDERequestIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER

// Set up reporting of evaluated requests.
#define LANGUAGE_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
template<>                                                                     \
inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,                \
                            const RequestType &request) {                      \
  ++stats.getFrontendCounters().RequestType;                                   \
}
#include "language/IDE/IDERequestIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_IDE_REQUESTS_H
