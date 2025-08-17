//===--- Attributes.cpp ---------------------------------------------------===//
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
// This file implements the AttributeCommonInfo interface.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/Attributes.h"
#include "language/Core/Basic/AttrSubjectMatchRules.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/ParsedAttrInfo.h"
#include "language/Core/Basic/SimpleTypoCorrection.h"
#include "language/Core/Basic/TargetInfo.h"

#include "toolchain/ADT/StringSwitch.h"

using namespace language::Core;

static StringRef canonicalizeScopeName(StringRef Name) {
  // Normalize the scope name, but only for gnu and clang attributes.
  if (Name == "__gnu__")
    return "gnu";

  if (Name == "_Clang")
    return "clang";

  return Name;
}

static StringRef canonicalizeAttrName(StringRef Name) {
  // Normalize the attribute name, __foo__ becomes foo.
  if (Name.size() >= 4 && Name.starts_with("__") && Name.ends_with("__"))
    return Name.substr(2, Name.size() - 4);

  return Name;
}

static int hasAttributeImpl(AttributeCommonInfo::Syntax Syntax, StringRef Name,
                            StringRef ScopeName, const TargetInfo &Target,
                            const LangOptions &LangOpts) {
#include "language/Core/Basic/AttrHasAttributeImpl.inc"
  return 0;
}

int language::Core::hasAttribute(AttributeCommonInfo::Syntax Syntax, StringRef ScopeName,
                        StringRef Name, const TargetInfo &Target,
                        const LangOptions &LangOpts, bool CheckPlugins) {
  ScopeName = canonicalizeScopeName(ScopeName);
  Name = canonicalizeAttrName(Name);

  // As a special case, look for the omp::sequence and omp::directive
  // attributes. We support those, but not through the typical attribute
  // machinery that goes through TableGen. We support this in all OpenMP modes
  // so long as double square brackets are enabled.
  //
  // Other OpenMP attributes (e.g. [[omp::assume]]) are handled via the
  // regular attribute parsing machinery.
  if (LangOpts.OpenMP && ScopeName == "omp" &&
      (Name == "directive" || Name == "sequence"))
    return 1;

  int res = hasAttributeImpl(Syntax, Name, ScopeName, Target, LangOpts);
  if (res)
    return res;

  if (CheckPlugins) {
    // Check if any plugin provides this attribute.
    for (auto &Ptr : getAttributePluginInstances())
      if (Ptr->hasSpelling(Syntax, Name))
        return 1;
  }

  return 0;
}

int language::Core::hasAttribute(AttributeCommonInfo::Syntax Syntax,
                        const IdentifierInfo *Scope, const IdentifierInfo *Attr,
                        const TargetInfo &Target, const LangOptions &LangOpts,
                        bool CheckPlugins) {
  return hasAttribute(Syntax, Scope ? Scope->getName() : "", Attr->getName(),
                      Target, LangOpts, CheckPlugins);
}

int language::Core::hasAttribute(AttributeCommonInfo::Syntax Syntax,
                        const IdentifierInfo *Scope, const IdentifierInfo *Attr,
                        const TargetInfo &Target, const LangOptions &LangOpts) {
  return hasAttribute(Syntax, Scope, Attr, Target, LangOpts,
                      /*CheckPlugins=*/true);
}

const char *attr::getSubjectMatchRuleSpelling(attr::SubjectMatchRule Rule) {
  switch (Rule) {
#define ATTR_MATCH_RULE(NAME, SPELLING, IsAbstract)                            \
  case attr::NAME:                                                             \
    return SPELLING;
#include "language/Core/Basic/AttrSubMatchRulesList.inc"
  }
  toolchain_unreachable("Invalid subject match rule");
}

static StringRef
normalizeAttrScopeName(StringRef ScopeName,
                       AttributeCommonInfo::Syntax SyntaxUsed) {
  if (SyntaxUsed == AttributeCommonInfo::AS_CXX11 ||
      SyntaxUsed == AttributeCommonInfo::AS_C23)
    return canonicalizeScopeName(ScopeName);

  return ScopeName;
}

static StringRef
normalizeAttrScopeName(const IdentifierInfo *ScopeName,
                       AttributeCommonInfo::Syntax SyntaxUsed) {
  if (ScopeName)
    return normalizeAttrScopeName(ScopeName->getName(), SyntaxUsed);
  return "";
}

static StringRef normalizeAttrName(StringRef AttrName,
                                   StringRef NormalizedScopeName,
                                   AttributeCommonInfo::Syntax SyntaxUsed) {
  // Normalize the attribute name, __foo__ becomes foo. This is only allowable
  // for GNU attributes, and attributes using the double square bracket syntax.
  bool ShouldNormalize =
      SyntaxUsed == AttributeCommonInfo::AS_GNU ||
      ((SyntaxUsed == AttributeCommonInfo::AS_CXX11 ||
        SyntaxUsed == AttributeCommonInfo::AS_C23) &&
       (NormalizedScopeName.empty() || NormalizedScopeName == "gnu" ||
        NormalizedScopeName == "clang"));

  if (ShouldNormalize)
    return canonicalizeAttrName(AttrName);

  return AttrName;
}

StringRef AttributeCommonInfo::getNormalizedScopeName() const {
  return normalizeAttrScopeName(getScopeName(), getSyntax());
}

StringRef
AttributeCommonInfo::getNormalizedAttrName(StringRef ScopeName) const {
  return normalizeAttrName(getAttrName()->getName(), ScopeName, getSyntax());
}

bool AttributeCommonInfo::isGNUScope() const {
  return AttrScope.isValid() && (AttrScope.getName()->isStr("gnu") ||
                                 AttrScope.getName()->isStr("__gnu__"));
}

bool AttributeCommonInfo::isClangScope() const {
  return AttrScope.isValid() && (AttrScope.getName()->isStr("clang") ||
                                 AttrScope.getName()->isStr("_Clang"));
}

#include "language/Core/Sema/AttrParsedAttrKinds.inc"

static SmallString<64> normalizeName(StringRef AttrName, StringRef ScopeName,
                                     AttributeCommonInfo::Syntax SyntaxUsed) {
  std::string StrAttrName = SyntaxUsed == AttributeCommonInfo::AS_HLSLAnnotation
                                ? AttrName.lower()
                                : AttrName.str();
  SmallString<64> FullName = ScopeName;
  if (!ScopeName.empty()) {
    assert(SyntaxUsed == AttributeCommonInfo::AS_CXX11 ||
           SyntaxUsed == AttributeCommonInfo::AS_C23);
    FullName += "::";
  }
  FullName += StrAttrName;
  return FullName;
}

static SmallString<64> normalizeName(const IdentifierInfo *Name,
                                     const IdentifierInfo *Scope,
                                     AttributeCommonInfo::Syntax SyntaxUsed) {
  StringRef ScopeName = normalizeAttrScopeName(Scope, SyntaxUsed);
  StringRef AttrName =
      normalizeAttrName(Name->getName(), ScopeName, SyntaxUsed);
  return normalizeName(AttrName, ScopeName, SyntaxUsed);
}

AttributeCommonInfo::Kind
AttributeCommonInfo::getParsedKind(const IdentifierInfo *Name,
                                   const IdentifierInfo *ScopeName,
                                   Syntax SyntaxUsed) {
  return ::getAttrKind(normalizeName(Name, ScopeName, SyntaxUsed), SyntaxUsed);
}

AttributeCommonInfo::AttrArgsInfo
AttributeCommonInfo::getCXX11AttrArgsInfo(const IdentifierInfo *Name) {
  StringRef AttrName = normalizeAttrName(
      Name->getName(), /*NormalizedScopeName*/ "", Syntax::AS_CXX11);
#define CXX11_ATTR_ARGS_INFO
  return toolchain::StringSwitch<AttributeCommonInfo::AttrArgsInfo>(AttrName)
#include "language/Core/Basic/CXX11AttributeInfo.inc"
      .Default(AttributeCommonInfo::AttrArgsInfo::None);
#undef CXX11_ATTR_ARGS_INFO
}

std::string AttributeCommonInfo::getNormalizedFullName() const {
  return static_cast<std::string>(
      normalizeName(getAttrName(), getScopeName(), getSyntax()));
}

std::string
AttributeCommonInfo::getNormalizedFullName(StringRef ScopeName,
                                           StringRef AttrName) const {
  return static_cast<std::string>(
      normalizeName(AttrName, ScopeName, getSyntax()));
}

SourceRange AttributeCommonInfo::getNormalizedRange() const {
  return hasScope() ? SourceRange(AttrScope.getNameLoc(), AttrRange.getEnd())
                    : AttrRange;
}

static AttributeCommonInfo::Scope
getScopeFromNormalizedScopeName(StringRef ScopeName) {
  return toolchain::StringSwitch<AttributeCommonInfo::Scope>(ScopeName)
      .Case("", AttributeCommonInfo::Scope::NONE)
      .Case("clang", AttributeCommonInfo::Scope::CLANG)
      .Case("gnu", AttributeCommonInfo::Scope::GNU)
      .Case("gsl", AttributeCommonInfo::Scope::GSL)
      .Case("hlsl", AttributeCommonInfo::Scope::HLSL)
      .Case("vk", AttributeCommonInfo::Scope::VK)
      .Case("msvc", AttributeCommonInfo::Scope::MSVC)
      .Case("omp", AttributeCommonInfo::Scope::OMP)
      .Case("riscv", AttributeCommonInfo::Scope::RISCV);
}

unsigned AttributeCommonInfo::calculateAttributeSpellingListIndex() const {
  // Both variables will be used in tablegen generated
  // attribute spell list index matching code.
  auto Syntax = static_cast<AttributeCommonInfo::Syntax>(getSyntax());
  StringRef ScopeName = normalizeAttrScopeName(getScopeName(), Syntax);
  StringRef Name =
      normalizeAttrName(getAttrName()->getName(), ScopeName, Syntax);
  AttributeCommonInfo::Scope ComputedScope =
      getScopeFromNormalizedScopeName(ScopeName);

#include "language/Core/Sema/AttrSpellingListIndex.inc"
}

#define ATTR_NAME(NAME) NAME,
static constexpr const char *AttrSpellingList[] = {
#include "language/Core/Basic/AttributeSpellingList.inc"
};

#define ATTR_SCOPE_NAME(SCOPE_NAME) SCOPE_NAME,
static constexpr const char *AttrScopeSpellingList[] = {
#include "language/Core/Basic/AttributeSpellingList.inc"
};

std::optional<StringRef>
AttributeCommonInfo::tryGetCorrectedScopeName(StringRef ScopeName) const {
  if (ScopeName.size() > 0 &&
      !toolchain::is_contained(AttrScopeSpellingList, ScopeName)) {
    SimpleTypoCorrection STC(ScopeName);
    for (const auto &Scope : AttrScopeSpellingList)
      STC.add(Scope);

    if (auto CorrectedScopeName = STC.getCorrection())
      return CorrectedScopeName;
  }
  return std::nullopt;
}

std::optional<StringRef> AttributeCommonInfo::tryGetCorrectedAttrName(
    StringRef ScopeName, StringRef AttrName, const TargetInfo &Target,
    const LangOptions &LangOpts) const {
  if (!toolchain::is_contained(AttrSpellingList, AttrName)) {
    SimpleTypoCorrection STC(AttrName);
    for (const auto &Attr : AttrSpellingList)
      STC.add(Attr);

    if (auto CorrectedAttrName = STC.getCorrection()) {
      if (hasAttribute(getSyntax(), ScopeName, *CorrectedAttrName, Target,
                       LangOpts,
                       /*CheckPlugins=*/true))
        return CorrectedAttrName;
    }
  }
  return std::nullopt;
}
