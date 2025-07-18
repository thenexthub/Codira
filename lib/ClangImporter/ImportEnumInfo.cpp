//===--- ImportEnumInfo.cpp - Information about importable Clang enums ----===//
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
// This file provides EnumInfo, which describes a Clang enum ready to be
// imported
//
//===----------------------------------------------------------------------===//

#include "ClangAdapter.h"
#include "ImportEnumInfo.h"
#include "ImporterImpl.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/StringExtras.h"
#include "language/Parse/Lexer.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/Preprocessor.h"

#include "toolchain/ADT/Statistic.h"
#define DEBUG_TYPE "Enum Info"
STATISTIC(EnumInfoNumCacheHits, "# of times the enum info cache was hit");
STATISTIC(EnumInfoNumCacheMisses, "# of times the enum info cache was missed");

using namespace language;
using namespace importer;

/// Find the last extensibility attribute on \p decl as arranged by source
/// location...unless there's an API note, in which case that one wins.
///
/// This is not what Clang will do, but it's more useful for us since CF_ENUM
/// already has enum_extensibility(open) in it.
static clang::EnumExtensibilityAttr *
getBestExtensibilityAttr(clang::Preprocessor &pp, const clang::EnumDecl *decl) {
  clang::EnumExtensibilityAttr *bestSoFar = nullptr;
  const clang::SourceManager &sourceMgr = pp.getSourceManager();
  for (auto *next : decl->specific_attrs<clang::EnumExtensibilityAttr>()) {
    if (next->getLocation().isInvalid()) {
      // This is from API notes -- use it!
      return next;
    }

    if (!bestSoFar ||
        sourceMgr.isBeforeInTranslationUnit(bestSoFar->getLocation(),
                                            next->getLocation())) {
      bestSoFar = next;
    }
  }
  return bestSoFar;
}

/// Classify the given Clang enumeration to describe how to import it.
void EnumInfo::classifyEnum(const clang::EnumDecl *decl,
                            clang::Preprocessor &pp) {
  assert(decl);
  clang::PrettyStackTraceDecl trace(decl, clang::SourceLocation(),
                                    pp.getSourceManager(), "classifying");
  assert(decl->isThisDeclarationADefinition());

  // Anonymous enumerations simply get mapped to constants of the
  // underlying type of the enum, because there is no way to conjure up a
  // name for the Codira type.
  if (!decl->hasNameForLinkage()) {
    // If this enum comes from a typedef, we can find a name.
    const clang::Type *underlyingType = getUnderlyingType(decl);
    if (!isa<clang::TypedefType>(underlyingType) ||
        // If the typedef is available in Codira, the user will get ambiguity.
        // It also means they may not have intended this API to be imported like this.
        !importer::isUnavailableInCodira(
            cast<clang::TypedefType>(underlyingType)->getDecl(),
            nullptr, true)) {
      kind = EnumKind::Constants;
      return;
    }
  }

  // First, check for attributes that denote the classification.
  if (auto domainAttr = decl->getAttr<clang::NSErrorDomainAttr>()) {
    kind = EnumKind::NonFrozenEnum;
    nsErrorDomain = domainAttr->getErrorDomain()->getName();
  }
  if (decl->hasAttr<clang::FlagEnumAttr>()) {
    kind = EnumKind::Options;
    return;
  }
  if (auto *attr = getBestExtensibilityAttr(pp, decl)) {
    if (attr->getExtensibility() == clang::EnumExtensibilityAttr::Closed)
      kind = EnumKind::FrozenEnum;
    else
      kind = EnumKind::NonFrozenEnum;
    return;
  }
  if (!nsErrorDomain.empty())
    return;

  if (decl->isScoped()) {
    kind = EnumKind::NonFrozenEnum;
    return;
  }

  // If API notes have /removed/ a FlagEnum or EnumExtensibility attribute,
  // then we don't need to check the macros.
  for (auto *attr : decl->specific_attrs<clang::CodiraVersionedAdditionAttr>()) {
    if (!attr->getIsReplacedByActive())
      continue;
    if (isa<clang::FlagEnumAttr>(attr->getAdditionalAttr()) ||
        isa<clang::EnumExtensibilityAttr>(attr->getAdditionalAttr())) {
      kind = EnumKind::Unknown;
      return;
    }
  }

  // Was the enum declared using *_ENUM or *_OPTIONS?
  // FIXME: Stop using these once flag_enum and enum_extensibility
  // have been adopted everywhere, or at least relegate them to Codira 4 mode
  // only.
  auto loc = decl->getBeginLoc();
  if (loc.isMacroID()) {
    StringRef MacroName = pp.getImmediateMacroName(loc);
    if (MacroName == "CF_ENUM" || MacroName == "__CF_NAMED_ENUM" ||
        MacroName == "OBJC_ENUM" || MacroName == "LANGUAGE_ENUM" ||
        MacroName == "LANGUAGE_ENUM_NAMED") {
      kind = EnumKind::NonFrozenEnum;
      return;
    }
    if (MacroName == "CF_OPTIONS" || MacroName == "OBJC_OPTIONS" ||
        MacroName == "LANGUAGE_OPTIONS") {
      kind = EnumKind::Options;
      return;
    }
  }

  // Hardcode a particular annoying case in the OS X headers.
  if (decl->getName() == "DYLD_BOOL") {
    kind = EnumKind::FrozenEnum;
    return;
  }

  // Fall back to the 'Unknown' path.
  kind = EnumKind::Unknown;
}

/// Returns the common prefix of two strings at camel-case word granularity.
///
/// For example, given "NSFooBar" and "NSFooBas", returns "NSFoo"
/// (not "NSFooBa"). The returned StringRef is a slice of the "a" argument.
///
/// If either string has a non-identifier character immediately after the
/// prefix, \p followedByNonIdentifier will be set to \c true. If both strings
/// have identifier characters after the prefix, \p followedByNonIdentifier will
/// be set to \c false. Otherwise, \p followedByNonIdentifier will not be
/// changed from its initial value.
///
/// This is used to derive the common prefix of enum constants so we can elide
/// it from the Codira interface.
StringRef importer::getCommonWordPrefix(StringRef a, StringRef b,
                                        bool &followedByNonIdentifier) {
  auto aWords = camel_case::getWords(a), bWords = camel_case::getWords(b);
  auto aI = aWords.begin(), aE = aWords.end(), bI = bWords.begin(),
       bE = bWords.end();

  unsigned prevLength = 0;
  unsigned prefixLength = 0;
  for (; aI != aE && bI != bE; ++aI, ++bI) {
    if (*aI != *bI) {
      followedByNonIdentifier = false;
      break;
    }

    prevLength = prefixLength;
    prefixLength = aI.getPosition() + aI->size();
  }

  // Avoid creating a prefix where the rest of the string starts with a number.
  if ((aI != aE && !Lexer::isIdentifier(*aI)) ||
      (bI != bE && !Lexer::isIdentifier(*bI))) {
    followedByNonIdentifier = true;
    prefixLength = prevLength;
  }

  return a.slice(0, prefixLength);
}

/// Returns the common word-prefix of two strings, allowing the second string
/// to be a common English plural form of the first.
///
/// For example, given "NSProperty" and "NSProperties", the full "NSProperty"
/// is returned. Given "NSMagicArmor" and "NSMagicArmory", only
/// "NSMagic" is returned.
///
/// The "-s", "-es", and "-ies" patterns cover every plural NS_OPTIONS name
/// in Cocoa and Cocoa Touch.
///
/// \see getCommonWordPrefix
StringRef importer::getCommonPluralPrefix(StringRef singular,
                                          StringRef plural) {
  assert(!plural.empty());

  if (singular.empty())
    return singular;

  bool ignored;
  StringRef commonPrefix = getCommonWordPrefix(singular, plural, ignored);
  if (commonPrefix.size() == singular.size() || plural.back() != 's')
    return commonPrefix;

  StringRef leftover = singular.substr(commonPrefix.size());
  StringRef firstLeftoverWord = camel_case::getFirstWord(leftover);
  StringRef commonPrefixPlusWord =
      singular.substr(0, commonPrefix.size() + firstLeftoverWord.size());

  // Is the plural string just "[singular]s"?
  plural = plural.drop_back();
  if (plural.ends_with(firstLeftoverWord))
    return commonPrefixPlusWord;

  if (plural.empty() || plural.back() != 'e')
    return commonPrefix;

  // Is the plural string "[singular]es"?
  plural = plural.drop_back();
  if (plural.ends_with(firstLeftoverWord))
    return commonPrefixPlusWord;

  if (plural.empty() || !(plural.back() == 'i' && singular.back() == 'y'))
    return commonPrefix;

  // Is the plural string "[prefix]ies" and the singular "[prefix]y"?
  plural = plural.drop_back();
  firstLeftoverWord = firstLeftoverWord.drop_back();
  if (plural.ends_with(firstLeftoverWord))
    return commonPrefixPlusWord;

  return commonPrefix;
}

const clang::Type *importer::getUnderlyingType(const clang::EnumDecl *decl) {
  return importer::desugarIfElaborated(decl->getIntegerType().getTypePtr());
}

ImportedType importer::findOptionSetEnum(clang::QualType type,
                                         ClangImporter::Implementation &Impl) {
  auto typedefType = dyn_cast<clang::TypedefType>(type);
  if (!typedefType || !Impl.isUnavailableInCodira(typedefType->getDecl()))
    // If this isn't a typedef, or it is a typedef that is available in Codira,
    // then this definitely isn't used for {CF,NS}_OPTIONS.
    return ImportedType();

  if (Impl.CodiraContext.LangOpts.EnableCXXInterop &&
      !isCFOptionsMacro(typedefType->getDecl(), Impl.getClangPreprocessor())) {
    return ImportedType();
  }

  auto clangEnum = findAnonymousEnumForTypedef(Impl.CodiraContext, typedefType);
  if (!clangEnum)
    return ImportedType();

  // Assert that the typedef has the same underlying integer representation as
  // the enum we think it assigns a type name to.
  //
  // If these fails, it means that we need a stronger predicate for
  // determining the relationship between an enum and typedef.
  if (auto *tdEnum =
          dyn_cast<clang::EnumType>(typedefType->getCanonicalTypeInternal())) {
    ASSERT(clangEnum.value()->getIntegerType()->getCanonicalTypeInternal() ==
           tdEnum->getDecl()->getIntegerType()->getCanonicalTypeInternal());
  } else {
    ASSERT(clangEnum.value()->getIntegerType()->getCanonicalTypeInternal() ==
           typedefType->getCanonicalTypeInternal());
  }

  if (auto *languageEnum = Impl.importDecl(*clangEnum, Impl.CurrentVersion))
    return {cast<TypeDecl>(languageEnum)->getDeclaredInterfaceType(), false};

  return ImportedType();
}

/// Determine the prefix to be stripped from the names of the enum constants
/// within the given enum.
void EnumInfo::determineConstantNamePrefix(const clang::EnumDecl *decl) {
  switch (getKind()) {
  case EnumKind::NonFrozenEnum:
  case EnumKind::FrozenEnum:
  case EnumKind::Options:
    // Enums are mapped to Codira enums, Options to Codira option sets, both
    // of which attempt prefix-stripping.
    break;

  case EnumKind::Constants:
  case EnumKind::Unknown:
    // Nothing to do.
    return;
  }

  // If there are no enumerators, there is no prefix to compute.
  auto ec = decl->enumerator_begin(), ecEnd = decl->enumerator_end();
  if (ec == ecEnd)
    return;

  // Determine whether the given enumerator is non-deprecated and has no
  // specifically-provided name.
  auto isNonDeprecatedWithoutCustomName = [](
      const clang::EnumConstantDecl *elem) -> bool {
    if (elem->hasAttr<clang::CodiraNameAttr>())
      return false;

    toolchain::VersionTuple maxVersion{~0U, ~0U, ~0U};
    switch (elem->getAvailability(nullptr, maxVersion)) {
    case clang::AR_Available:
    case clang::AR_NotYetIntroduced:
      for (auto attr : elem->attrs()) {
        if (auto annotate = dyn_cast<clang::AnnotateAttr>(attr)) {
          if (annotate->getAnnotation() == "language1_unavailable")
            return false;
        }
        if (auto avail = dyn_cast<clang::AvailabilityAttr>(attr)) {
          if (avail->getPlatform()->getName() == "language")
            return false;
        }
      }
      return true;

    case clang::AR_Deprecated:
    case clang::AR_Unavailable:
      return false;
    }

    toolchain_unreachable("Invalid AvailabilityAttr.");
  };

  // Move to the first non-deprecated enumerator, or non-language_name'd
  // enumerator, if present.
  auto firstNonDeprecated =
      std::find_if(ec, ecEnd, isNonDeprecatedWithoutCustomName);
  bool hasNonDeprecated = (firstNonDeprecated != ecEnd);
  if (hasNonDeprecated) {
    ec = firstNonDeprecated;
  } else {
    // Advance to the first case without a custom name, deprecated or not.
    while (ec != ecEnd && (*ec)->hasAttr<clang::CodiraNameAttr>())
      ++ec;
    if (ec == ecEnd) {
      return;
    }
  }

  // Compute the common prefix.
  StringRef commonPrefix = (*ec)->getName();
  bool followedByNonIdentifier = false;
  for (++ec; ec != ecEnd; ++ec) {
    // Skip deprecated or language_name'd enumerators.
    const clang::EnumConstantDecl *elem = *ec;
    if (hasNonDeprecated) {
      if (!isNonDeprecatedWithoutCustomName(elem))
        continue;
    } else {
      if (elem->hasAttr<clang::CodiraNameAttr>())
        continue;
    }

    commonPrefix = getCommonWordPrefix(commonPrefix, elem->getName(),
                                       followedByNonIdentifier);
    if (commonPrefix.empty())
      break;
  }

  if (!commonPrefix.empty()) {
    StringRef checkPrefix = commonPrefix;

    // Account for the 'kConstant' naming convention on enumerators.
    if (checkPrefix[0] == 'k') {
      bool canDropK;
      if (checkPrefix.size() >= 2)
        canDropK = clang::isUppercase(checkPrefix[1]);
      else
        canDropK = !followedByNonIdentifier;

      if (canDropK)
        checkPrefix = checkPrefix.drop_front();
    }

    // Don't use importFullName() here, we want to ignore the language_name
    // and language_private attributes.
    StringRef enumNameStr;
    // If there's no name, this must be typedef. So use the typedef's name.
    if (!decl->hasNameForLinkage()) {
      const clang::Type *underlyingType = getUnderlyingType(decl);
      auto typedefDecl = cast<clang::TypedefType>(underlyingType)->getDecl();
      enumNameStr = typedefDecl->getName();
    } else {
      enumNameStr = decl->getName();
    }

    if (enumNameStr.empty())
      enumNameStr = decl->getTypedefNameForAnonDecl()->getName();
    assert(!enumNameStr.empty() && "should have been classified as Constants");

    StringRef commonWithEnum = getCommonPluralPrefix(checkPrefix, enumNameStr);
    size_t delta = commonPrefix.size() - checkPrefix.size();

    // Account for the 'EnumName_Constant' convention on enumerators.
    if (commonWithEnum.size() < checkPrefix.size() &&
        checkPrefix[commonWithEnum.size()] == '_' && !followedByNonIdentifier) {
      delta += 1;
    }

    commonPrefix = commonPrefix.slice(0, commonWithEnum.size() + delta);
  }

  constantNamePrefix = commonPrefix;
}

EnumInfo EnumInfoCache::getEnumInfo(const clang::EnumDecl *decl) {
  auto iter = enumInfos.find(decl);
  if (iter != enumInfos.end()) {
    ++EnumInfoNumCacheHits;
    return iter->second;
  }
  ++EnumInfoNumCacheMisses;
  EnumInfo enumInfo(decl, clangPP);
  enumInfos[decl] = enumInfo;
  return enumInfo;
}
