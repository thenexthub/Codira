//===--- ClangImporterRequests.h - Clang Importer Requests ------*- C++ -*-===//
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
//  This file defines clang-importer requests.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CLANG_IMPORTER_REQUESTS_H
#define LANGUAGE_CLANG_IMPORTER_REQUESTS_H

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/EvaluatorDependencies.h"
#include "language/AST/Identifier.h"
#include "language/AST/NameLookup.h"
#include "language/AST/SimpleRequest.h"
#include "language/Basic/Statistic.h"
#include "language/ClangImporter/ClangImporter.h"
#include "clang/AST/Type.h"
#include "clang/Basic/Specifiers.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/TinyPtrVector.h"

namespace language {
class Decl;
class DeclName;
class EnumDecl;
enum class ExplicitSafety;

/// The input type for a clang direct lookup request.
struct ClangDirectLookupDescriptor final {
  Decl *decl;
  const clang::Decl *clangDecl;
  DeclName name;

  ClangDirectLookupDescriptor(Decl *decl, const clang::Decl *clangDecl,
                              DeclName name)
      : decl(decl), clangDecl(clangDecl), name(name) {}

  friend toolchain::hash_code hash_value(const ClangDirectLookupDescriptor &desc) {
    return toolchain::hash_combine(desc.name, desc.decl, desc.clangDecl);
  }

  friend bool operator==(const ClangDirectLookupDescriptor &lhs,
                         const ClangDirectLookupDescriptor &rhs) {
    return lhs.name == rhs.name && lhs.decl == rhs.decl &&
           lhs.clangDecl == rhs.clangDecl;
  }

  friend bool operator!=(const ClangDirectLookupDescriptor &lhs,
                         const ClangDirectLookupDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out,
                    const ClangDirectLookupDescriptor &desc);
SourceLoc extractNearestSourceLoc(const ClangDirectLookupDescriptor &desc);

/// This matches CodiraLookupTable::SingleEntry;
using SingleEntry = toolchain::PointerUnion<clang::NamedDecl *, clang::MacroInfo *,
                                       clang::ModuleMacro *>;
/// Uses the appropriate CodiraLookupTable to find a set of clang decls given
/// their name.
class ClangDirectLookupRequest
    : public SimpleRequest<ClangDirectLookupRequest,
                           SmallVector<SingleEntry, 4>(
                               ClangDirectLookupDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  SmallVector<SingleEntry, 4> evaluate(Evaluator &evaluator,
                                       ClangDirectLookupDescriptor desc) const;
};

/// The input type for a namespace member lookup request.
struct CXXNamespaceMemberLookupDescriptor final {
  EnumDecl *namespaceDecl;
  DeclName name;

  CXXNamespaceMemberLookupDescriptor(EnumDecl *namespaceDecl, DeclName name)
      : namespaceDecl(namespaceDecl), name(name) {
    assert(isa<clang::NamespaceDecl>(namespaceDecl->getClangDecl()));
  }

  friend toolchain::hash_code
  hash_value(const CXXNamespaceMemberLookupDescriptor &desc) {
    return toolchain::hash_combine(desc.name, desc.namespaceDecl);
  }

  friend bool operator==(const CXXNamespaceMemberLookupDescriptor &lhs,
                         const CXXNamespaceMemberLookupDescriptor &rhs) {
    return lhs.name == rhs.name && lhs.namespaceDecl == rhs.namespaceDecl;
  }

  friend bool operator!=(const CXXNamespaceMemberLookupDescriptor &lhs,
                         const CXXNamespaceMemberLookupDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out,
                    const CXXNamespaceMemberLookupDescriptor &desc);
SourceLoc
extractNearestSourceLoc(const CXXNamespaceMemberLookupDescriptor &desc);

/// Uses ClangDirectLookup to find a named member inside of the given namespace.
class CXXNamespaceMemberLookup
    : public SimpleRequest<CXXNamespaceMemberLookup,
                           TinyPtrVector<ValueDecl *>(
                               CXXNamespaceMemberLookupDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  TinyPtrVector<ValueDecl *>
  evaluate(Evaluator &evaluator, CXXNamespaceMemberLookupDescriptor desc) const;
};

/// The input type for a record member lookup request.
///
/// These lookups may be requested recursively in the case of inheritance,
/// for which we separately keep track of the derived class where we started
/// looking (startDecl) and the access level for the current inheritance.
struct ClangRecordMemberLookupDescriptor final {
  NominalTypeDecl *recordDecl;     // Where we are currently looking
  NominalTypeDecl *inheritingDecl; // Where we started looking from
  DeclName name;                   // What we are looking for
  ClangInheritanceInfo inheritance;

  ClangRecordMemberLookupDescriptor(NominalTypeDecl *recordDecl, DeclName name)
      : recordDecl(recordDecl), inheritingDecl(recordDecl), name(name),
        inheritance() {
    assert(isa<clang::RecordDecl>(recordDecl->getClangDecl()));
  }

  friend toolchain::hash_code
  hash_value(const ClangRecordMemberLookupDescriptor &desc) {
    return toolchain::hash_combine(desc.name, desc.recordDecl, desc.inheritingDecl,
                              desc.inheritance);
  }

  friend bool operator==(const ClangRecordMemberLookupDescriptor &lhs,
                         const ClangRecordMemberLookupDescriptor &rhs) {
    return lhs.name == rhs.name && lhs.recordDecl == rhs.recordDecl &&
           lhs.inheritingDecl == rhs.inheritingDecl &&
           lhs.inheritance == rhs.inheritance;
  }

  friend bool operator!=(const ClangRecordMemberLookupDescriptor &lhs,
                         const ClangRecordMemberLookupDescriptor &rhs) {
    return !(lhs == rhs);
  }

private:
  friend class ClangRecordMemberLookup;

  // This private constructor should only be used in ClangRecordMemberLookup,
  // for recursively traversing base classes that inheritingDecl inherites from.
  ClangRecordMemberLookupDescriptor(NominalTypeDecl *recordDecl, DeclName name,
                                    NominalTypeDecl *inheritingDecl,
                                    ClangInheritanceInfo inheritance)
      : recordDecl(recordDecl), inheritingDecl(inheritingDecl), name(name),
        inheritance(inheritance) {
    assert(isa<clang::RecordDecl>(recordDecl->getClangDecl()));
    assert(isa<clang::CXXRecordDecl>(inheritingDecl->getClangDecl()));
    assert(inheritance.isInheriting() &&
           "recursive calls should indicate inheritance");
    assert(recordDecl != inheritingDecl &&
           "recursive calls should lookup elsewhere");
  }
};

void simple_display(toolchain::raw_ostream &out,
                    const ClangRecordMemberLookupDescriptor &desc);
SourceLoc
extractNearestSourceLoc(const ClangRecordMemberLookupDescriptor &desc);

/// Uses ClangDirectLookup to find a named member inside of the given record.
class ClangRecordMemberLookup
    : public SimpleRequest<ClangRecordMemberLookup,
                           TinyPtrVector<ValueDecl *>(
                               ClangRecordMemberLookupDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  TinyPtrVector<ValueDecl *>
  evaluate(Evaluator &evaluator, ClangRecordMemberLookupDescriptor desc) const;
};

/// The input type for a clang category lookup request.
struct ClangCategoryLookupDescriptor final {
  const ClassDecl *classDecl;
  Identifier categoryName;

  ClangCategoryLookupDescriptor(const ClassDecl *classDecl,
                                Identifier categoryName)
      : classDecl(classDecl), categoryName(categoryName) {}

  friend toolchain::hash_code hash_value(const ClangCategoryLookupDescriptor &desc) {
    return toolchain::hash_combine(desc.classDecl, desc.categoryName);
  }

  friend bool operator==(const ClangCategoryLookupDescriptor &lhs,
                         const ClangCategoryLookupDescriptor &rhs) {
    return lhs.classDecl == rhs.classDecl
               && lhs.categoryName == rhs.categoryName;
  }

  friend bool operator!=(const ClangCategoryLookupDescriptor &lhs,
                         const ClangCategoryLookupDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out,
                    const ClangCategoryLookupDescriptor &desc);
SourceLoc extractNearestSourceLoc(const ClangCategoryLookupDescriptor &desc);

/// Given a Codira class, find the imported Codira decl(s) representing the
/// \c \@interface with the given category name. An empty \c categoryName
/// represents the main interface for the class.
///
/// That is, this request will return one of:
///
/// \li a single \c language::ExtensionDecl backed by a \c clang::ObjCCategoryDecl
/// \li a \c language::ClassDecl backed by a \c clang::ObjCInterfaceDecl, plus
///     zero or more \c language::ExtensionDecl s backed by 
///     \c clang::ObjCCategoryDecl s (representing ObjC class extensions).
/// \li an empty list if the class is not imported from Clang or it does not
///     have a category by that name.
class ClangCategoryLookupRequest
    : public SimpleRequest<ClangCategoryLookupRequest,
                           TinyPtrVector<Decl *>(ClangCategoryLookupDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
 TinyPtrVector<Decl *> evaluate(Evaluator &evaluator,
                                ClangCategoryLookupDescriptor desc) const;
};

/// Links an \c \@_objcImplementation decl to the imported declaration(s) that
/// it implements.
///
/// There is usually a 1:1 correspondence between interfaces and
/// implementations, except that a class's main implementation implements
/// both its main interface and any class extension interfaces. In this
/// situation, the main class is always the first decl in \c interfaceDecls.
struct ObjCInterfaceAndImplementation final {
  toolchain::TinyPtrVector<Decl *> interfaceDecls;
  Decl *implementationDecl;

  ObjCInterfaceAndImplementation(toolchain::TinyPtrVector<Decl *> interfaceDecls,
                                 Decl *implementationDecl)
      : interfaceDecls(interfaceDecls), implementationDecl(implementationDecl)
  {
    assert(!interfaceDecls.empty() && implementationDecl &&
           "interface and implementation are both non-null");
  }

  ObjCInterfaceAndImplementation()
      : interfaceDecls(), implementationDecl(nullptr) {}

  bool empty() const {
    return interfaceDecls.empty();
  }

  friend toolchain::hash_code
  hash_value(const ObjCInterfaceAndImplementation &pair) {
    return hash_combine(toolchain::hash_combine_range(pair.interfaceDecls.begin(),
                                                 pair.interfaceDecls.end()),
                        pair.implementationDecl);
  }

  friend bool operator==(const ObjCInterfaceAndImplementation &lhs,
                         const ObjCInterfaceAndImplementation &rhs) {
    return lhs.interfaceDecls == rhs.interfaceDecls
               && lhs.implementationDecl == rhs.implementationDecl;
  }

  friend bool operator!=(const ObjCInterfaceAndImplementation &lhs,
                         const ObjCInterfaceAndImplementation &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out,
                    const ObjCInterfaceAndImplementation &desc);
SourceLoc extractNearestSourceLoc(const ObjCInterfaceAndImplementation &desc);

/// Given a \c Decl , determine if it is an implementation with separate
/// interfaces imported from ObjC (or vice versa) and if so, return all of the
/// declarations involved in this relationship. Otherwise return an empty value.
///
/// We perform this lookup in both directions using a single request because
/// we want to cache the relationship on both sides to avoid duplicating work.
class ObjCInterfaceAndImplementationRequest
    : public SimpleRequest<ObjCInterfaceAndImplementationRequest,
                           ObjCInterfaceAndImplementation(Decl *),
                           RequestFlags::SeparatelyCached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  ObjCInterfaceAndImplementation
  evaluate(Evaluator &evaluator, Decl *decl) const;

 public:
   // Separate caching.
   bool isCached() const { return true; }
   std::optional<ObjCInterfaceAndImplementation> getCachedResult() const;
   void cacheResult(ObjCInterfaceAndImplementation value) const;
};

enum class CxxRecordSemanticsKind {
  Trivial,
  Owned,
  MoveOnly,
  Reference,
  Iterator,
  // A record that is either not copyable/movable or not destructible.
  MissingLifetimeOperation,
  // A record that has no copy and no move operations
  UnavailableConstructors,
  // A C++ record that represents a Codira class type exposed to C++ from Codira.
  CodiraClassType
};

struct CxxRecordSemanticsDescriptor final {
  const clang::RecordDecl *decl;
  ASTContext &ctx;
  ClangImporter::Implementation *importerImpl;

  CxxRecordSemanticsDescriptor(const clang::RecordDecl *decl, ASTContext &ctx,
                               ClangImporter::Implementation *importerImpl)
      : decl(decl), ctx(ctx), importerImpl(importerImpl) {}

  friend toolchain::hash_code hash_value(const CxxRecordSemanticsDescriptor &desc) {
    return toolchain::hash_combine(desc.decl);
  }

  friend bool operator==(const CxxRecordSemanticsDescriptor &lhs,
                         const CxxRecordSemanticsDescriptor &rhs) {
    return lhs.decl == rhs.decl;
  }

  friend bool operator!=(const CxxRecordSemanticsDescriptor &lhs,
                         const CxxRecordSemanticsDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out, CxxRecordSemanticsDescriptor desc);
SourceLoc extractNearestSourceLoc(CxxRecordSemanticsDescriptor desc);

/// What pattern does this C++ API fit? Uses attributes such as
/// import_owned and import_reference to determine the pattern.
///
/// Do not evaluate this request before importing has started. For example, it
/// is OK to invoke this request when importing a decl, but it is not OK to
/// evaluate this request when importing names. This is because when importing
/// names, Clang sema has not yet defined implicit special members, so the
/// results will be flakey/incorrect.
class CxxRecordSemantics
    : public SimpleRequest<CxxRecordSemantics,
                           CxxRecordSemanticsKind(CxxRecordSemanticsDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };

private:
  friend SimpleRequest;

  // Evaluation.
  CxxRecordSemanticsKind evaluate(Evaluator &evaluator,
                                  CxxRecordSemanticsDescriptor) const;
};

/// Does this C++ record represent a Codira type.
class CxxRecordAsCodiraType
    : public SimpleRequest<CxxRecordAsCodiraType,
                           ValueDecl *(CxxRecordSemanticsDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };

private:
  friend SimpleRequest;

  ValueDecl *evaluate(Evaluator &evaluator, CxxRecordSemanticsDescriptor) const;
};

struct SafeUseOfCxxDeclDescriptor final {
  const clang::Decl *decl;

  SafeUseOfCxxDeclDescriptor(const clang::Decl *decl) : decl(decl) {}

  friend toolchain::hash_code hash_value(const SafeUseOfCxxDeclDescriptor &desc) {
    return toolchain::hash_combine(desc.decl);
  }

  friend bool operator==(const SafeUseOfCxxDeclDescriptor &lhs,
                         const SafeUseOfCxxDeclDescriptor &rhs) {
    return lhs.decl == rhs.decl;
  }

  friend bool operator!=(const SafeUseOfCxxDeclDescriptor &lhs,
                         const SafeUseOfCxxDeclDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out, SafeUseOfCxxDeclDescriptor desc);
SourceLoc extractNearestSourceLoc(SafeUseOfCxxDeclDescriptor desc);

class IsSafeUseOfCxxDecl
    : public SimpleRequest<IsSafeUseOfCxxDecl, bool(SafeUseOfCxxDeclDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };

private:
  friend SimpleRequest;

  // Evaluation.
  bool evaluate(Evaluator &evaluator, SafeUseOfCxxDeclDescriptor desc) const;
};

enum class CustomRefCountingOperationKind { retain, release };

struct CustomRefCountingOperationDescriptor final {
  const ClassDecl *decl;
  CustomRefCountingOperationKind kind;

  CustomRefCountingOperationDescriptor(const ClassDecl *decl,
                                       CustomRefCountingOperationKind kind)
      : decl(decl), kind(kind) {}

  friend toolchain::hash_code
  hash_value(const CustomRefCountingOperationDescriptor &desc) {
    return toolchain::hash_combine(desc.decl, desc.kind);
  }

  friend bool operator==(const CustomRefCountingOperationDescriptor &lhs,
                         const CustomRefCountingOperationDescriptor &rhs) {
    return lhs.decl == rhs.decl && lhs.kind == rhs.kind;
  }

  friend bool operator!=(const CustomRefCountingOperationDescriptor &lhs,
                         const CustomRefCountingOperationDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out,
                    CustomRefCountingOperationDescriptor desc);
SourceLoc extractNearestSourceLoc(CustomRefCountingOperationDescriptor desc);

struct CustomRefCountingOperationResult {
  enum CustomRefCountingOperationResultKind {
    noAttribute,
    tooManyAttributes,
    immortal,
    notFound,
    tooManyFound,
    foundOperation
  };

  CustomRefCountingOperationResultKind kind;
  ValueDecl *operation;
  std::string name;
};

class CustomRefCountingOperation
    : public SimpleRequest<CustomRefCountingOperation,
                           CustomRefCountingOperationResult(
                               CustomRefCountingOperationDescriptor),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

  // Caching
  bool isCached() const { return true; }

  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };

private:
  friend SimpleRequest;

  // Evaluation.
  CustomRefCountingOperationResult
  evaluate(Evaluator &evaluator,
           CustomRefCountingOperationDescriptor desc) const;
};

enum class CxxEscapability { Escapable, NonEscapable, Unknown };

struct EscapabilityLookupDescriptor final {
  const clang::Type *type;
  ClangImporter::Implementation *impl;
  // Only explicitly ~Escapable annotated types are considered ~Escapable.
  // This is for backward compatibility, so we continue to import aggregates
  // containing pointers as Escapable types.
  bool annotationOnly = true;

  friend toolchain::hash_code hash_value(const EscapabilityLookupDescriptor &desc) {
    return toolchain::hash_combine(desc.type);
  }

  friend bool operator==(const EscapabilityLookupDescriptor &lhs,
                         const EscapabilityLookupDescriptor &rhs) {
    return lhs.type == rhs.type && lhs.annotationOnly == rhs.annotationOnly;
  }

  friend bool operator!=(const EscapabilityLookupDescriptor &lhs,
                         const EscapabilityLookupDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

class ClangTypeEscapability
    : public SimpleRequest<ClangTypeEscapability,
                           CxxEscapability(EscapabilityLookupDescriptor),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

  bool isCached() const { return true; }

private:
  friend SimpleRequest;

  CxxEscapability evaluate(Evaluator &evaluator,
                           EscapabilityLookupDescriptor desc) const;
};

void simple_display(toolchain::raw_ostream &out, EscapabilityLookupDescriptor desc);
SourceLoc extractNearestSourceLoc(EscapabilityLookupDescriptor desc);

struct CxxDeclExplicitSafetyDescriptor final {
  const clang::Decl *decl;
  bool isClass;

  CxxDeclExplicitSafetyDescriptor(const clang::Decl *decl, bool isClass)
      : decl(decl), isClass(isClass) {}

  friend toolchain::hash_code
  hash_value(const CxxDeclExplicitSafetyDescriptor &desc) {
    return toolchain::hash_combine(desc.decl, desc.isClass);
  }

  friend bool operator==(const CxxDeclExplicitSafetyDescriptor &lhs,
                         const CxxDeclExplicitSafetyDescriptor &rhs) {
    return lhs.decl == rhs.decl && lhs.isClass == rhs.isClass;
  }

  friend bool operator!=(const CxxDeclExplicitSafetyDescriptor &lhs,
                         const CxxDeclExplicitSafetyDescriptor &rhs) {
    return !(lhs == rhs);
  }
};

void simple_display(toolchain::raw_ostream &out,
                    CxxDeclExplicitSafetyDescriptor desc);
SourceLoc extractNearestSourceLoc(CxxDeclExplicitSafetyDescriptor desc);

/// Determine the safety of the given Clang declaration.
class ClangDeclExplicitSafety
    : public SimpleRequest<ClangDeclExplicitSafety,
                           ExplicitSafety(CxxDeclExplicitSafetyDescriptor),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
  bool isCached() const;

private:
  friend SimpleRequest;

  // Evaluation.
  ExplicitSafety evaluate(Evaluator &evaluator,
                          CxxDeclExplicitSafetyDescriptor desc) const;
};

#define LANGUAGE_TYPEID_ZONE ClangImporter
#define LANGUAGE_TYPEID_HEADER "language/ClangImporter/ClangImporterTypeIDZone.def"
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
#include "language/ClangImporter/ClangImporterTypeIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_CLANG_IMPORTER_REQUESTS_H

