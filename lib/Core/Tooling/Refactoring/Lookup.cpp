//===--- Lookup.cpp - Framework for clang refactoring tools ---------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  This file defines helper methods for clang tools performing name lookup.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/Refactoring/Lookup.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclarationName.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/ADT/SmallVector.h"
using namespace language::Core;
using namespace language::Core::tooling;

// Gets all namespaces that \p Context is in as a vector (ignoring anonymous
// namespaces). The inner namespaces come before outer namespaces in the vector.
// For example, if the context is in the following namespace:
//    `namespace a { namespace b { namespace c ( ... ) } }`,
// the vector will be `{c, b, a}`.
static toolchain::SmallVector<const NamespaceDecl *, 4>
getAllNamedNamespaces(const DeclContext *Context) {
  toolchain::SmallVector<const NamespaceDecl *, 4> Namespaces;
  auto GetNextNamedNamespace = [](const DeclContext *Context) {
    // Look past non-namespaces and anonymous namespaces on FromContext.
    while (Context && (!isa<NamespaceDecl>(Context) ||
                       cast<NamespaceDecl>(Context)->isAnonymousNamespace()))
      Context = Context->getParent();
    return Context;
  };
  for (Context = GetNextNamedNamespace(Context); Context != nullptr;
       Context = GetNextNamedNamespace(Context->getParent()))
    Namespaces.push_back(cast<NamespaceDecl>(Context));
  return Namespaces;
}

// Returns true if the context in which the type is used and the context in
// which the type is declared are the same semantical namespace but different
// lexical namespaces.
static bool
usingFromDifferentCanonicalNamespace(const DeclContext *FromContext,
                                     const DeclContext *UseContext) {
  // We can skip anonymous namespace because:
  // 1. `FromContext` and `UseContext` must be in the same anonymous namespaces
  // since referencing across anonymous namespaces is not possible.
  // 2. If `FromContext` and `UseContext` are in the same anonymous namespace,
  // the function will still return `false` as expected.
  toolchain::SmallVector<const NamespaceDecl *, 4> FromNamespaces =
      getAllNamedNamespaces(FromContext);
  toolchain::SmallVector<const NamespaceDecl *, 4> UseNamespaces =
      getAllNamedNamespaces(UseContext);
  // If `UseContext` has fewer level of nested namespaces, it cannot be in the
  // same canonical namespace as the `FromContext`.
  if (UseNamespaces.size() < FromNamespaces.size())
    return false;
  unsigned Diff = UseNamespaces.size() - FromNamespaces.size();
  auto FromIter = FromNamespaces.begin();
  // Only compare `FromNamespaces` with namespaces in `UseNamespaces` that can
  // collide, i.e. the top N namespaces where N is the number of namespaces in
  // `FromNamespaces`.
  auto UseIter = UseNamespaces.begin() + Diff;
  for (; FromIter != FromNamespaces.end() && UseIter != UseNamespaces.end();
       ++FromIter, ++UseIter) {
    // Literally the same namespace, not a collision.
    if (*FromIter == *UseIter)
      return false;
    // Now check the names. If they match we have a different canonical
    // namespace with the same name.
    if (cast<NamespaceDecl>(*FromIter)->getDeclName() ==
        cast<NamespaceDecl>(*UseIter)->getDeclName())
      return true;
  }
  assert(FromIter == FromNamespaces.end() && UseIter == UseNamespaces.end());
  return false;
}

static StringRef getBestNamespaceSubstr(const DeclContext *DeclA,
                                        StringRef NewName,
                                        bool HadLeadingColonColon) {
  while (true) {
    while (DeclA && !isa<NamespaceDecl>(DeclA))
      DeclA = DeclA->getParent();

    // Fully qualified it is! Leave :: in place if it's there already.
    if (!DeclA)
      return HadLeadingColonColon ? NewName : NewName.substr(2);

    // Otherwise strip off redundant namespace qualifications from the new name.
    // We use the fully qualified name of the namespace and remove that part
    // from NewName if it has an identical prefix.
    std::string NS =
        "::" + cast<NamespaceDecl>(DeclA)->getQualifiedNameAsString() + "::";
    if (NewName.consume_front(NS))
      return NewName;

    // No match yet. Strip of a namespace from the end of the chain and try
    // again. This allows to get optimal qualifications even if the old and new
    // decl only share common namespaces at a higher level.
    DeclA = DeclA->getParent();
  }
}

// Adds more scope specifier to the spelled name until the spelling is not
// ambiguous. A spelling is ambiguous if the resolution of the symbol is
// ambiguous. For example, if QName is "::y::bar", the spelling is "y::bar", and
// context contains a nested namespace "a::y", then "y::bar" can be resolved to
// ::a::y::bar in the context, which can cause compile error.
// FIXME: consider using namespaces.
static std::string disambiguateSpellingInScope(StringRef Spelling,
                                               StringRef QName,
                                               const DeclContext &UseContext,
                                               SourceLocation UseLoc) {
  assert(QName.starts_with("::"));
  assert(QName.ends_with(Spelling));
  if (Spelling.starts_with("::"))
    return std::string(Spelling);

  auto UnspelledSpecifier = QName.drop_back(Spelling.size());
  toolchain::SmallVector<toolchain::StringRef, 2> UnspelledScopes;
  UnspelledSpecifier.split(UnspelledScopes, "::", /*MaxSplit=*/-1,
                           /*KeepEmpty=*/false);

  toolchain::SmallVector<const NamespaceDecl *, 4> EnclosingNamespaces =
      getAllNamedNamespaces(&UseContext);
  auto &AST = UseContext.getParentASTContext();
  StringRef TrimmedQName = QName.substr(2);
  const auto &SM = UseContext.getParentASTContext().getSourceManager();
  UseLoc = SM.getSpellingLoc(UseLoc);

  auto IsAmbiguousSpelling = [&](const toolchain::StringRef CurSpelling) {
    if (CurSpelling.starts_with("::"))
      return false;
    // Lookup the first component of Spelling in all enclosing namespaces
    // and check if there is any existing symbols with the same name but in
    // different scope.
    StringRef Head = CurSpelling.split("::").first;
    for (const auto *NS : EnclosingNamespaces) {
      auto LookupRes = NS->lookup(DeclarationName(&AST.Idents.get(Head)));
      if (!LookupRes.empty()) {
        for (const NamedDecl *Res : LookupRes)
          // If `Res` is not visible in `UseLoc`, we don't consider it
          // ambiguous. For example, a reference in a header file should not be
          // affected by a potentially ambiguous name in some file that includes
          // the header.
          if (!TrimmedQName.starts_with(Res->getQualifiedNameAsString()) &&
              SM.isBeforeInTranslationUnit(
                  SM.getSpellingLoc(Res->getLocation()), UseLoc))
            return true;
      }
    }
    return false;
  };

  // Add more qualifiers until the spelling is not ambiguous.
  std::string Disambiguated = std::string(Spelling);
  while (IsAmbiguousSpelling(Disambiguated)) {
    if (UnspelledScopes.empty()) {
      Disambiguated = "::" + Disambiguated;
    } else {
      Disambiguated = (UnspelledScopes.back() + "::" + Disambiguated).str();
      UnspelledScopes.pop_back();
    }
  }
  return Disambiguated;
}

std::string tooling::replaceNestedName(NestedNameSpecifier Use,
                                       SourceLocation UseLoc,
                                       const DeclContext *UseContext,
                                       const NamedDecl *FromDecl,
                                       StringRef ReplacementString) {
  assert(ReplacementString.starts_with("::") &&
         "Expected fully-qualified name!");

  // We can do a raw name replacement when we are not inside the namespace for
  // the original class/function and it is not in the global namespace.  The
  // assumption is that outside the original namespace we must have a using
  // statement that makes this work out and that other parts of this refactor
  // will automatically fix using statements to point to the new class/function.
  // However, if the `FromDecl` is a class forward declaration, the reference is
  // still considered as referring to the original definition, so we can't do a
  // raw name replacement in this case.
  const bool class_name_only = !Use;
  const bool in_global_namespace =
      isa<TranslationUnitDecl>(FromDecl->getDeclContext());
  const bool is_class_forward_decl =
      isa<CXXRecordDecl>(FromDecl) &&
      !cast<CXXRecordDecl>(FromDecl)->isCompleteDefinition();
  if (class_name_only && !in_global_namespace && !is_class_forward_decl &&
      !usingFromDifferentCanonicalNamespace(FromDecl->getDeclContext(),
                                            UseContext)) {
    auto Pos = ReplacementString.rfind("::");
    return std::string(Pos != StringRef::npos
                           ? ReplacementString.substr(Pos + 2)
                           : ReplacementString);
  }
  // We did not match this because of a using statement, so we will need to
  // figure out how good a namespace match we have with our destination type.
  // We work backwards (from most specific possible namespace to least
  // specific).
  StringRef Suggested = getBestNamespaceSubstr(UseContext, ReplacementString,
                                               Use.isFullyQualified());

  return disambiguateSpellingInScope(Suggested, ReplacementString, *UseContext,
                                     UseLoc);
}
