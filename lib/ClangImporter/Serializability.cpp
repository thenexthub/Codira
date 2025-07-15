//===--- Serializability.cpp - Codira serializability of Clang AST refs ----===//
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
// This file implements support for working with StableSerializationPaths
// and determining whether references to Clang declarations and types are
// serializable.
//
// The expectation here is that the same basic predicates are
// interesting  for both binary (.codemodule) and textual
// (.codeinterface) serialization.  For textual serialization, the
// key question is whether a printed representation will round-trip.
//
//===----------------------------------------------------------------------===//

#include "ImporterImpl.h"
#include "language/Basic/Assertions.h"
#include "language/ClangImporter/CodiraAbstractBasicWriter.h"

using namespace language;

using ExternalPath = StableSerializationPath::ExternalPath;

static bool isSameDecl(const clang::Decl *lhs, const clang::Decl *rhs) {
  return lhs == rhs || lhs->getCanonicalDecl() == rhs->getCanonicalDecl();
}

namespace {
class SerializationPathFinder {
  ClangImporter::Implementation &Impl;
public:
  SerializationPathFinder(ClangImporter::Implementation &impl) : Impl(impl) {}

  StableSerializationPath find(const clang::Decl *decl) {
    // We can't do anything with non-NamedDecl declarations.
    auto named = dyn_cast<clang::NamedDecl>(decl);
    if (!named) return StableSerializationPath();

    if (decl->isFromASTFile()) {
      return findImportedPath(named);
    }

    // If the declaration isn't from an AST file, it might be something that
    // we built automatically when exporting a Codira type.
    if (auto languageDecl =
          Impl.CodiraContext.getCodiraDeclForExportedClangDecl(decl))
      return languageDecl;

    // Allow serialization for non-modular headers as well, with the hope that
    // we find the same header when doing unqualified lookup during
    // deserialization.
    return findImportedPath(named);
  }

private:
  Identifier getIdentifier(const clang::IdentifierInfo *clangIdent) {
    return Impl.CodiraContext.getIdentifier(clangIdent->getName());
  }

  StableSerializationPath findImportedPath(const clang::NamedDecl *decl) {
    // We've almost certainly imported this declaration, look for it.
    std::optional<Decl *> languageDeclOpt =
        Impl.importDeclCached(decl, Impl.CurrentVersion);
    if (languageDeclOpt.has_value() && languageDeclOpt.value()) {
      auto languageDecl = languageDeclOpt.value();
      // The serialization code doesn't allow us to cross-reference
      // typealias declarations directly.  We could fix that, but it's
      // easier to just avoid doing so and fall into the external-path code.
      if (!isa<TypeAliasDecl>(languageDecl)) {
        // Only accept this declaration if it round-trips.
        if (auto languageClangDecl = languageDecl->getClangDecl())
          if (isSameDecl(decl, languageClangDecl))
            return languageDecl;
      }
    }

    // Otherwise, check to see if it's something we can easily find.
    ExternalPath path;
    if (findExternalPath(decl, path))
      return std::move(path);

    // Otherwise we have no way to find it.
    return StableSerializationPath();
  }

  bool findExternalPath(const clang::NamedDecl *decl, ExternalPath &path) {
    if (auto tag = dyn_cast<clang::TagDecl>(decl))
      return findExternalPath(tag, path);
    if (auto alias = dyn_cast<clang::TypedefNameDecl>(decl))
      return findExternalPath(alias, path);
    if (auto proto = dyn_cast<clang::ObjCProtocolDecl>(decl))
      return findExternalPath(proto, path);
    if (auto iface = dyn_cast<clang::ObjCInterfaceDecl>(decl))
      return findExternalPath(iface, path);
    return false;
  }

  bool findExternalPath(const clang::TagDecl *decl, ExternalPath &path) {
    // We can't handle class template specializations right now.
    if (isa<clang::ClassTemplateSpecializationDecl>(decl))
      return false;

    // Named tags are straightforward.
    if (auto name = decl->getIdentifier()) {
      if (!findExternalPath(decl->getDeclContext(), path)) return false;
      path.add(decl->isEnum() ? ExternalPath::Enum : ExternalPath::Record,
               getIdentifier(name));
      return true;
    }

    // We can handle anonymous tags if they're defined in an obvious
    // position in a typedef.
    if (auto alias = decl->getTypedefNameForAnonDecl()) {
      auto aliasTag = alias->getAnonDeclWithTypedefName(/*any*/true);
      if (aliasTag && isSameDecl(decl, aliasTag)) {
        if (!findExternalPath(alias, path)) return false;
        path.add(ExternalPath::TypedefAnonDecl, Identifier());
        return true;
      }
    }

    // Otherwise we're stuck.
    return false;
  }

  bool findExternalPath(const clang::TypedefNameDecl *decl,
                        ExternalPath &path) {
    auto name = decl->getIdentifier();
    if (!name) return false;
    if (!findExternalPath(decl->getDeclContext(), path)) return false;
    path.add(ExternalPath::Typedef, getIdentifier(name));
    return true;
  }

  bool findExternalPath(const clang::ObjCProtocolDecl *decl,
                        ExternalPath &path) {
    auto name = decl->getIdentifier();
    if (!name) return false;
    path.add(ExternalPath::ObjCProtocol, getIdentifier(name));
    return true;
  }

  bool findExternalPath(const clang::ObjCInterfaceDecl *decl,
                        ExternalPath &path) {
    auto name = decl->getIdentifier();
    if (!name) return false;
    path.add(ExternalPath::ObjCInterface, getIdentifier(name));
    return true;
  }

  bool findExternalPath(const clang::DeclContext *dc, ExternalPath &path) {
    // If we've reached the translation unit, we're done.
    if (isa<clang::TranslationUnitDecl>(dc))
      return true;

    // Linkage specifications don't contribute to the path.
    if (isa<clang::LinkageSpecDecl>(dc))
      return findExternalPath(dc->getParent(), path);

    // Handle namespaces.
    if (auto ns = dyn_cast<clang::NamespaceDecl>(dc)) {
      // Don't try to handle anonymous namespaces.
      auto name = ns->getIdentifier();
      if (!name) return false;

      // Drill to the parent.
      if (!findExternalPath(dc->getParent(), path)) return false;

      path.Path.push_back({ExternalPath::Namespace, getIdentifier(name)});
      return true;
    }

    // Handle types.
    if (auto tag = dyn_cast<clang::TagDecl>(dc))
      return findExternalPath(tag, path);

    // Can't handle anything else.
    return false;
  }
};
} // end anonymous namespace


StableSerializationPath
ClangImporter::findStableSerializationPath(const clang::Decl *decl) const {
  return Impl.findStableSerializationPath(decl);
}

StableSerializationPath
ClangImporter::Implementation::findStableSerializationPath(
                                                    const clang::Decl *decl) {
  return SerializationPathFinder(*this).find(decl);
}

const clang::Decl *
ClangImporter::resolveStableSerializationPath(
                                  const StableSerializationPath &path) const {
  if (!path) return nullptr;

  if (path.isCodiraDecl()) {
    return path.getCodiraDecl()->getClangDecl();
  }

  auto &extpath = path.getExternalPath();
  auto &clangCtx = getClangASTContext();

  const clang::Decl *decl = nullptr;

  // Perform a lookup in the current context (`decl` if set, and
  // otherwise the translation unit).
  auto lookup = [&](Identifier name) -> clang::DeclContext::lookup_result {    
    if (name.empty()) return clang::DeclContext::lookup_result();

    const clang::DeclContext *dc;
    if (decl) {
      dc = dyn_cast<clang::DeclContext>(decl);
      if (!dc) return clang::DeclContext::lookup_result();
    } else {
      dc = clangCtx.getTranslationUnitDecl();
    }

    auto ident = &clangCtx.Idents.get(name.str());
    return dc->lookup(ident);
  };

  for (auto step : extpath.Path) {
    // Handle the non-lookup steps here.
    if (step.first == ExternalPath::TypedefAnonDecl) {
      if (auto alias = dyn_cast_or_null<clang::TypedefNameDecl>(decl))
        return alias->getAnonDeclWithTypedefName();
      return nullptr;
    }

    assert(ExternalPath::requiresIdentifier(step.first) &&
           "should've handled all non-lookup kinds above");

    const clang::Decl *resultDecl = nullptr;
    for (auto lookupDecl : lookup(step.second)) {
      auto isAcceptable = [](const clang::Decl *decl,
                             ExternalPath::ComponentKind kind) {
        switch (kind) {
        case ExternalPath::Record:
          return isa<clang::RecordDecl>(decl);
        case ExternalPath::Enum:
          return isa<clang::EnumDecl>(decl);
        case ExternalPath::Namespace:
          return isa<clang::NamespaceDecl>(decl);
        case ExternalPath::Typedef:
          return isa<clang::TypedefNameDecl>(decl);
        case ExternalPath::ObjCInterface:
          return isa<clang::ObjCInterfaceDecl>(decl);
        case ExternalPath::ObjCProtocol:
          return isa<clang::ObjCProtocolDecl>(decl);
        case ExternalPath::TypedefAnonDecl:
          toolchain_unreachable("should have been filtered above");
        }
        toolchain_unreachable("bad kind");
      };

      // Ignore unacceptable declarations.
      if (!isAcceptable(lookupDecl, step.first))
        continue;
  
      // Bail out if we find multiple matching declarations.
      // TODO: make an effort to filter by the target module?
      if (resultDecl && !isSameDecl(resultDecl, lookupDecl))
        return nullptr;

      resultDecl = lookupDecl;
    }

    // Bail out if lookup found nothing.
    if (!resultDecl) return nullptr;

    decl = resultDecl;
  }

  return decl;
}

namespace {
  /// The logic here for the supported cases must match the logic in
  /// ClangToCodiraBasicWriter in Serialization.cpp.
  struct ClangTypeSerializationChecker :
      DataStreamBasicWriter<ClangTypeSerializationChecker> {
    ClangImporter::Implementation &Impl;
    bool IsSerializable = true;

    ClangTypeSerializationChecker(ClangImporter::Implementation &impl)
      : DataStreamBasicWriter<ClangTypeSerializationChecker>(
          impl.getClangASTContext()),
        Impl(impl) {}

    void writeUInt64(uint64_t value) {}
    void writeIdentifier(const clang::IdentifierInfo *ident) {}
    void writeStmtRef(const clang::Stmt *stmt) {
      if (stmt != nullptr)
        IsSerializable = false;
    }
    void writeDeclRef(const clang::Decl *decl) {
      if (decl && !Impl.findStableSerializationPath(decl))
        IsSerializable = false;
    }
    void writeSourceLocation(clang::SourceLocation loc) {
      // If a source location is written into a type, it's likely to be
      // something like the location of a VLA which we shouldn't simply
      // replace with a meaningless location.
      if (loc.isValid())
        IsSerializable = false;
    }

    void writeAttr(const clang::Attr *attr) {}

    // CountAttributedType is a clang type representing a pointer with
    // a "counted_by" type attribute and DynamicRangePointerType
    // is representing a "__ended_by" type attribute.
    // TypeCoupledDeclRefInfo is used to hold information of a declaration
    // referenced from an expression argument of "__counted_by(expr)" or
    // "__ended_by(expr)".
    // Leave it non-serializable for now as we currently don't import
    // these types into Codira.
    void writeTypeCoupledDeclRefInfo(clang::TypeCoupledDeclRefInfo info) {
      toolchain_unreachable("TypeCoupledDeclRefInfo shouldn't be reached from language");
    }
  };
}

bool ClangImporter::isSerializable(const clang::Type *type,
                                   bool checkCanonical) const {
  return Impl.isSerializable(clang::QualType(type, 0), checkCanonical);
}

bool ClangImporter::Implementation::isSerializable(clang::QualType type,
                                                   bool checkCanonical) {
  if (checkCanonical)
    type = getClangASTContext().getCanonicalType(type);

  // Make a pass over the type as if we were serializing it, flagging
  // anything that we can't stably serialize.
  ClangTypeSerializationChecker checker(*this);
  checker.writeQualType(type);
  return checker.IsSerializable;
}
