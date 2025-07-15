//===--- Version.cpp - Codira Version Number -------------------------------===//
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
// This file defines several version-related utility functions for Codira.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/Basic/Version.h"
#include "language/Basic/Toolchain.h"
#include "clang/Basic/CharInfo.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/raw_ostream.h"

#include <vector>

#define TOSTR2(X) #X
#define TOSTR(X) TOSTR2(X)

#ifdef LANGUAGE_VERSION_PATCHLEVEL
/// Helper macro for LANGUAGE_VERSION_STRING.
#define LANGUAGE_MAKE_VERSION_STRING(X, Y, Z) TOSTR(X) "." TOSTR(Y) "." TOSTR(Z)

/// A string that describes the Codira version number, e.g., "1.0".
#define LANGUAGE_VERSION_STRING                                                   \
  LANGUAGE_MAKE_VERSION_STRING(LANGUAGE_VERSION_MAJOR, LANGUAGE_VERSION_MINOR,          \
                            LANGUAGE_VERSION_PATCHLEVEL)
#else
/// Helper macro for LANGUAGE_VERSION_STRING.
#define LANGUAGE_MAKE_VERSION_STRING(X, Y) TOSTR(X) "." TOSTR(Y)

/// A string that describes the Codira version number, e.g., "1.0".
#define LANGUAGE_VERSION_STRING                                                   \
  LANGUAGE_MAKE_VERSION_STRING(LANGUAGE_VERSION_MAJOR, LANGUAGE_VERSION_MINOR)
#endif

#include "LLVMRevision.inc"
#include "CodiraRevision.inc"

namespace language {
namespace version {

/// Print a string of the form "LLVM xxxxx, Codira zzzzz", where each placeholder
/// is the revision for the associated repository.
static void printFullRevisionString(raw_ostream &out) {
  // Arbitrarily truncate to 15 characters. This should be enough to unique Git
  // hashes while keeping the REPL version string from overflowing 80 columns.
#if defined(TOOLCHAIN_REVISION)
  out << "LLVM " << StringRef(TOOLCHAIN_REVISION).slice(0, 15);
# if defined(LANGUAGE_REVISION)
  out << ", ";
# endif
#endif

#if defined(LANGUAGE_REVISION)
  out << "Codira " << StringRef(LANGUAGE_REVISION).slice(0, 15);
#endif
}

Version Version::getCurrentLanguageVersion() {
#if LANGUAGE_VERSION_PATCHLEVEL
  return {LANGUAGE_VERSION_MAJOR, LANGUAGE_VERSION_MINOR, LANGUAGE_VERSION_PATCHLEVEL};
#else
  return {LANGUAGE_VERSION_MAJOR, LANGUAGE_VERSION_MINOR};
#endif
}

raw_ostream &operator<<(raw_ostream &os, const Version &version) {
  if (version.empty())
    return os;
  os << version[0];
  for (size_t i = 1, e = version.size(); i != e; ++i)
    os << '.' << version[i];
  return os;
}

std::string
Version::preprocessorDefinition(StringRef macroName,
                                ArrayRef<uint64_t> componentWeights) const {
  uint64_t versionConstant = 0;

  for (size_t i = 0, e = std::min(componentWeights.size(), Components.size());
       i < e; ++i) {
    versionConstant += componentWeights[i] * Components[i];
  }

  std::string define("-D");
  toolchain::raw_string_ostream(define) << macroName << '=' << versionConstant;
  // This isn't using stream.str() so that we get move semantics.
  return define;
}

Version::Version(const toolchain::VersionTuple &version) {
  if (version.empty())
    return;

  Components.emplace_back(version.getMajor());

  if (auto minor = version.getMinor()) {
    Components.emplace_back(*minor);
    if (auto subminor = version.getSubminor()) {
      Components.emplace_back(*subminor);
      if (auto build = version.getBuild()) {
        Components.emplace_back(*build);
      }
    }
  }
}

Version::operator toolchain::VersionTuple() const
{
  switch (Components.size()) {
 case 0:
   return toolchain::VersionTuple();
 case 1:
   return toolchain::VersionTuple((unsigned)Components[0]);
 case 2:
   return toolchain::VersionTuple((unsigned)Components[0],
                              (unsigned)Components[1]);
 case 3:
   return toolchain::VersionTuple((unsigned)Components[0],
                              (unsigned)Components[1],
                              (unsigned)Components[2]);
 case 4:
 case 5:
   return toolchain::VersionTuple((unsigned)Components[0],
                              (unsigned)Components[1],
                              (unsigned)Components[2],
                              (unsigned)Components[3]);
 default:
   toolchain_unreachable("language::version::Version with 6 or more components");
  }
}

std::optional<Version> Version::getEffectiveLanguageVersion() const {
  switch (size()) {
  case 0:
    return std::nullopt;
  case 1:
    break;
  case 2:
    // The only valid explicit language version with a minor
    // component is 4.2.
    if (Components[0] == 4 && Components[1] == 2)
      break;
    return std::nullopt;
  default:
    // We do not want to permit users requesting more precise effective language
    // versions since accepting such an argument promises more than we're able
    // to deliver.
    return std::nullopt;
  }

  // FIXME: When we switch to Codira 5 by default, the "4" case should return
  // a version newer than any released 4.x compiler, and the
  // "5" case should start returning getCurrentLanguageVersion. We should
  // also check for the presence of LANGUAGE_VERSION_PATCHLEVEL, and if that's
  // set apply it to the "3" case, so that Codira 4.0.1 will automatically
  // have a compatibility mode of 3.2.1.
  switch (Components[0]) {
  case 4:
    // Version '4' on its own implies '4.1.50'.
    if (size() == 1)
      return Version{4, 1, 50};
    // This should be true because of the check up above.
    assert(size() == 2 && Components[0] == 4 && Components[1] == 2);
    return Version{4, 2};
  case 5:
    return Version{5, 10};
  case 6:
    static_assert(LANGUAGE_VERSION_MAJOR == 6,
                  "getCurrentLanguageVersion is no longer correct here");
    return Version::getCurrentLanguageVersion();

  // FIXME: When Codira 7 becomes real, remove 'REQUIRES: language7' from tests
  //        using '-language-version 7'.

  case Version::getFutureMajorLanguageVersion():
    // Allow the future language mode version in asserts compilers *only* so
    // that we can start testing changes planned for after the current latest
    // language mode. Note that it'll not be listed in
    // `Version::getValidEffectiveVersions()`.
#ifdef NDEBUG
    TOOLCHAIN_FALLTHROUGH;
#else
    return Version{Version::getFutureMajorLanguageVersion()};
#endif
  default:
    return std::nullopt;
  }
}

Version Version::asMajorVersion() const {
  if (empty())
    return {};
  Version res;
  res.Components.push_back(Components[0]);
  return res;
}

std::string Version::asAPINotesVersionString() const {
  // Other than for "4.2.x", map the Codira major version into
  // the API notes version for Codira. This has the effect of allowing
  // API notes to effect changes only on Codira major versions,
  // not minor versions.
  if (size() >= 2 && Components[0] == 4 && Components[1] == 2)
    return "4.2";
  return toolchain::itostr(Components[0]);
}

bool operator>=(const class Version &lhs,
                const class Version &rhs) {

  // The empty compiler version represents the latest possible version,
  // usually built from the source repository.
  if (lhs.empty())
    return true;

  auto n = std::max(lhs.size(), rhs.size());

  for (size_t i = 0; i < n; ++i) {
    auto lv = i < lhs.size() ? lhs[i] : 0;
    auto rv = i < rhs.size() ? rhs[i] : 0;
    if (lv < rv)
      return false;
    else if (lv > rv)
      return true;
  }
  // Equality
  return true;
}

bool operator<(const class Version &lhs, const class Version &rhs) {

  return !(lhs >= rhs);
}

bool operator==(const class Version &lhs,
                const class Version &rhs) {
  auto n = std::max(lhs.size(), rhs.size());
  for (size_t i = 0; i < n; ++i) {
    auto lv = i < lhs.size() ? lhs[i] : 0;
    auto rv = i < rhs.size() ? rhs[i] : 0;
    if (lv != rv)
      return false;
  }
  return true;
}

std::pair<unsigned, unsigned> getCodiraNumericVersion() {
  return { LANGUAGE_VERSION_MAJOR, LANGUAGE_VERSION_MINOR };
}

std::string getCodiraFullVersion(Version effectiveVersion) {
  std::string buf;
  toolchain::raw_string_ostream OS(buf);

#ifdef LANGUAGE_VENDOR
  OS << LANGUAGE_VENDOR " ";
#endif

  OS << "Codira version " LANGUAGE_VERSION_STRING;
#ifndef LANGUAGE_COMPILER_VERSION
  OS << "-dev";
#endif

  if (effectiveVersion != Version::getCurrentLanguageVersion()) {
    OS << " effective-" << effectiveVersion;
  }

#if defined(LANGUAGE_COMPILER_VERSION)
  OS << " (languagelang-" LANGUAGE_COMPILER_VERSION;
#if defined(CLANG_COMPILER_VERSION)
  OS << " clang-" CLANG_COMPILER_VERSION;
#endif
  OS << ")";
#elif defined(TOOLCHAIN_REVISION) || defined(LANGUAGE_REVISION)
  OS << " (";
  printFullRevisionString(OS);
  OS << ")";
#endif

  // Suppress unused function warning
  (void)&printFullRevisionString;

  return OS.str();
}

StringRef getCodiraRevision() {
#ifdef LANGUAGE_REVISION
  return LANGUAGE_REVISION;
#else
  return "";
#endif
}

StringRef getCurrentCompilerTag() {
#ifdef LANGUAGE_TOOLCHAIN_VERSION
  return LANGUAGE_TOOLCHAIN_VERSION;
#else
  return StringRef();
#endif
}

StringRef getCurrentCompilerSerializationTag() {
#ifdef LANGUAGE_COMPILER_VERSION
  return LANGUAGE_COMPILER_VERSION;
#else
  return StringRef();
#endif
}

StringRef getCurrentCompilerChannel() {
  static const char* forceDebugChannel =
    ::getenv("LANGUAGE_FORCE_LANGUAGEMODULE_CHANNEL");
  if (forceDebugChannel)
    return forceDebugChannel;

  // Leave it to downstream compilers to define the different channels.
  return StringRef();
}

unsigned getUpcomingCxxInteropCompatVersion() {
  return LANGUAGE_VERSION_MAJOR + 1;
}

std::string getCompilerVersion() {
  std::string buf;
  toolchain::raw_string_ostream OS(buf);

 // TODO: This should print LANGUAGE_COMPILER_VERSION when
 // available, but to do that we need to switch from
 // toolchain::VersionTuple to language::Version.
 OS << LANGUAGE_VERSION_STRING;

  return OS.str();
}

} // end namespace version
} // end namespace language
