//===--- Platform.cpp - Implement platform-related helpers ----------------===//
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

#include "language/Basic/Assertions.h"
#include "language/Basic/Pack.h"
#include "language/Basic/Platform.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/TargetParser/Triple.h"
#include "toolchain/Support/VersionTuple.h"

using namespace language;

bool language::tripleIsiOSSimulator(const toolchain::Triple &triple) {
  return (triple.isiOS() &&
          !tripleIsMacCatalystEnvironment(triple) &&
          triple.isSimulatorEnvironment());
}

bool language::tripleIsAppleTVSimulator(const toolchain::Triple &triple) {
  return (triple.isTvOS() && triple.isSimulatorEnvironment());
}

bool language::tripleIsWatchSimulator(const toolchain::Triple &triple) {
  return (triple.isWatchOS() && triple.isSimulatorEnvironment());
}

bool language::tripleIsMacCatalystEnvironment(const toolchain::Triple &triple) {
  return triple.isiOS() && !triple.isTvOS() &&
      triple.getEnvironment() == toolchain::Triple::MacABI;
}

bool language::tripleIsVisionSimulator(const toolchain::Triple &triple) {
  return triple.isXROS() && triple.isSimulatorEnvironment();
}

bool language::tripleInfersSimulatorEnvironment(const toolchain::Triple &triple) {
  switch (triple.getOS()) {
  case toolchain::Triple::IOS:
  case toolchain::Triple::TvOS:
  case toolchain::Triple::WatchOS:
    return !triple.hasEnvironment() &&
        (triple.getArch() == toolchain::Triple::x86 ||
         triple.getArch() == toolchain::Triple::x86_64) &&
        !tripleIsMacCatalystEnvironment(triple);

  default:
    return false;
  }
}

bool language::triplesAreValidForZippering(const toolchain::Triple &target,
                                        const toolchain::Triple &targetVariant) {
  // The arch and vendor must match.
  if (target.getArchName() != targetVariant.getArchName() ||
      target.getArch() != targetVariant.getArch() ||
      target.getSubArch() != targetVariant.getSubArch() ||
      target.getVendor() != targetVariant.getVendor()) {
    return false;
  }

  // Allow a macOS target and an iOS-macabi target variant
  // This is typically the case when zippering a library originally
  // developed for macOS.
  if (target.isMacOSX() && tripleIsMacCatalystEnvironment(targetVariant)) {
    return true;
  }

  // Allow an iOS-macabi target and a macOS target variant. This would
  // be the case when zippering a library originally developed for
  // iOS.
  if (targetVariant.isMacOSX() && tripleIsMacCatalystEnvironment(target)) {
    return true;
  }

  return false;
}

const std::optional<toolchain::VersionTuple>
language::minimumAvailableOSVersionForTriple(const toolchain::Triple &triple) {
  if (triple.isMacOSX())
    return toolchain::VersionTuple(10, 10, 0);

  // Mac Catalyst was introduced with an iOS deployment target of 13.1.
  if (tripleIsMacCatalystEnvironment(triple))
    return toolchain::VersionTuple(13, 1);
  
  // Note: this must come before checking iOS since that returns true for
  // both iOS and tvOS.
  if (triple.isTvOS())
    return toolchain::VersionTuple(9, 0);

  if (triple.isiOS())
    return toolchain::VersionTuple(8, 0);

  if (triple.isWatchOS())
    return toolchain::VersionTuple(2, 0);

  if (triple.isXROS())
    return toolchain::VersionTuple(1, 0);

  return std::nullopt;
}

bool language::tripleRequiresRPathForCodiraLibrariesInOS(
    const toolchain::Triple &triple) {
  if (triple.isMacOSX()) {
    // macOS versions before 10.14.4 don't have Codira in the OS
    // (the linker still uses an rpath-based install name until 10.15).
    // macOS versions before 12.0 don't have _Concurrency in the OS.
    return triple.isMacOSXVersionLT(12, 0);
  }

  if (triple.isiOS()) {
    // iOS versions before 12.2 don't have Codira in the OS.
    // iOS versions before 15.0 don't have _Concurrency in the OS.
    return triple.isOSVersionLT(15, 0);
  }

  if (triple.isWatchOS()) {
    // watchOS versions before 5.2 don't have Codira in the OS.
    // watchOS versions before 8.0 don't have _Concurrency in the OS.
    return triple.isOSVersionLT(8, 0);
  }

  if (triple.isXROS()) {
    return triple.isOSVersionLT(1, 0);
  }

  // Other platforms don't have Codira installed as part of the OS by default.
  return false;
}

bool language::tripleBTCFIByDefaultInOpenBSD(const toolchain::Triple &triple) {
  return triple.isOSOpenBSD() && triple.getArch() == toolchain::Triple::aarch64;
}

DarwinPlatformKind language::getDarwinPlatformKind(const toolchain::Triple &triple) {
  if (triple.isiOS()) {
    if (triple.isTvOS()) {
      if (tripleIsAppleTVSimulator(triple))
        return DarwinPlatformKind::TvOSSimulator;
      return DarwinPlatformKind::TvOS;
    }

    if (tripleIsiOSSimulator(triple))
      return DarwinPlatformKind::IPhoneOSSimulator;

    return DarwinPlatformKind::IPhoneOS;
  }

  if (triple.isWatchOS()) {
    if (tripleIsWatchSimulator(triple))
      return DarwinPlatformKind::WatchOSSimulator;
    return DarwinPlatformKind::WatchOS;
  }

  if (triple.isMacOSX())
    return DarwinPlatformKind::MacOS;

  if (triple.isXROS()) {
    if (tripleIsVisionSimulator(triple))
      return DarwinPlatformKind::VisionOSSimulator;
    return DarwinPlatformKind::VisionOS;
  }

  toolchain_unreachable("Unsupported Darwin platform");
}

static StringRef getPlatformNameForDarwin(const DarwinPlatformKind platform) {
  switch (platform) {
  case DarwinPlatformKind::MacOS:
    return "macosx";
  case DarwinPlatformKind::IPhoneOS:
    return "iphoneos";
  case DarwinPlatformKind::IPhoneOSSimulator:
    return "iphonesimulator";
  case DarwinPlatformKind::TvOS:
    return "appletvos";
  case DarwinPlatformKind::TvOSSimulator:
    return "appletvsimulator";
  case DarwinPlatformKind::WatchOS:
    return "watchos";
  case DarwinPlatformKind::WatchOSSimulator:
    return "watchsimulator";
  case DarwinPlatformKind::VisionOS:
    return "xros";
  case DarwinPlatformKind::VisionOSSimulator:
    return "xrsimulator";
  }
  toolchain_unreachable("Unsupported Darwin platform");
}

StringRef language::getPlatformNameForTriple(const toolchain::Triple &triple) {
  switch (triple.getOS()) {
  case toolchain::Triple::AIX:
  case toolchain::Triple::AMDHSA:
  case toolchain::Triple::AMDPAL:
  case toolchain::Triple::BridgeOS:
  case toolchain::Triple::CUDA:
  case toolchain::Triple::DragonFly:
  case toolchain::Triple::DriverKit:
  case toolchain::Triple::ELFIAMCU:
  case toolchain::Triple::Emscripten:
  case toolchain::Triple::Fuchsia:
  case toolchain::Triple::HermitCore:
  case toolchain::Triple::Hurd:
  case toolchain::Triple::KFreeBSD:
  case toolchain::Triple::Lv2:
  case toolchain::Triple::Mesa3D:
  case toolchain::Triple::NaCl:
  case toolchain::Triple::NetBSD:
  case toolchain::Triple::NVCL:
  case toolchain::Triple::PS5:
  case toolchain::Triple::RTEMS:
  case toolchain::Triple::Serenity:
  case toolchain::Triple::ShaderModel:
  case toolchain::Triple::Solaris:
  case toolchain::Triple::Vulkan:
  case toolchain::Triple::ZOS:
    return "";
  case toolchain::Triple::Darwin:
  case toolchain::Triple::MacOSX:
  case toolchain::Triple::IOS:
  case toolchain::Triple::TvOS:
  case toolchain::Triple::WatchOS:
  case toolchain::Triple::XROS:
    return getPlatformNameForDarwin(getDarwinPlatformKind(triple));
  case toolchain::Triple::Linux:
    if (triple.isAndroid())
      return "android";
    else if (triple.isMusl()) {
      // The triple for linux-static is <arch>-language-linux-musl, to distinguish
      // it from a "normal" musl set-up (ala Alpine).
      if (triple.getVendor() == toolchain::Triple::Codira)
        return "linux-static";
      else
        return "musl";
    } else
      return "linux";
  case toolchain::Triple::FreeBSD:
    return "freebsd";
  case toolchain::Triple::OpenBSD:
    return "openbsd";
  case toolchain::Triple::Win32:
    switch (triple.getEnvironment()) {
    case toolchain::Triple::Cygnus:
      return "cygwin";
    case toolchain::Triple::GNU:
      return "mingw";
    case toolchain::Triple::MSVC:
    case toolchain::Triple::Itanium:
      return "windows";
    default:
      return "none";
    }
  case toolchain::Triple::PS4:
    return "ps4";
  case toolchain::Triple::Haiku:
    return "haiku";
  case toolchain::Triple::WASI:
    return "wasi";
  case toolchain::Triple::UnknownOS:
    return "none";
  case toolchain::Triple::UEFI:
  case toolchain::Triple::LiteOS:
    toolchain_unreachable("unsupported OS");
  }
  toolchain_unreachable("unsupported OS");
}

StringRef language::getMajorArchitectureName(const toolchain::Triple &Triple) {
  if (Triple.isOSLinux()) {
    switch (Triple.getSubArch()) {
    case toolchain::Triple::SubArchType::ARMSubArch_v7:
      return "armv7";
    case toolchain::Triple::SubArchType::ARMSubArch_v6:
      return "armv6";
    case toolchain::Triple::SubArchType::ARMSubArch_v5:
      return "armv5";
    default:
      break;
    }
  }

  if (Triple.isOSOpenBSD()) {
    if (Triple.getArchName() == "amd64") {
      return "x86_64";
    }
  }

  return Triple.getArchName();
}

// The code below is responsible for normalizing target triples into the form
// used to name target-specific languagemodule, languageinterface, and languagedoc files.
// If two triples have incompatible ABIs or can be distinguished by Codira #if
// declarations, they should normalize to different values.
//
// This code is only really used on platforms with toolchains supporting fat
// binaries (a single binary containing multiple architectures). On these
// platforms, this code should strip unnecessary details from target triple
// components and map synonyms to canonical values. Even values which don't need
// any special canonicalization should be documented here as comments.
//
// (Fallback behavior does not belong here; it should be implemented in code
// that calls this function, most importantly in SerializedModuleLoaderBase.)
//
// If you're trying to refer to this code to understand how Codira behaves and
// you're unfamiliar with LLVM internals, here's a cheat sheet for reading it:
//
// * toolchain::Triple is the type for a target name. It's a bit of a misnomer,
//   because it can contain three or four values: arch-vendor-os[-environment].
//
// * In .Cases and .Case, the last argument is the value the arguments before it
//   map to. That is, `.Cases("bar", "baz", "foo")` will return "foo" if it sees
//   "bar" or "baz".
//
// * std::optional is similar to a Codira Optional: it either contains a value
//   or represents the absence of one. `None` is equivalent to `nil`; leading
//   `*` is equivalent to trailing `!`; conversion to `bool` is a not-`None`
//   check.

static StringRef
getArchForAppleTargetSpecificModuleTriple(const toolchain::Triple &triple) {
  auto tripleArchName = triple.getArchName();

  return toolchain::StringSwitch<StringRef>(tripleArchName)
              .Cases("arm64", "aarch64", "arm64")
              .Cases("arm64_32", "aarch64_32", "arm64_32")
              .Cases("x86_64", "amd64", "x86_64")
              .Cases("i386", "i486", "i586", "i686", "i786", "i886", "i986",
                     "i386")
              .Cases("unknown", "", "unknown")
  // These values are also supported, but are handled by the default case below:
  //          .Case ("armv7s", "armv7s")
  //          .Case ("armv7k", "armv7k")
  //          .Case ("armv7", "armv7")
  //          .Case ("arm64e", "arm64e")
              .Default(tripleArchName);
}

static StringRef
getVendorForAppleTargetSpecificModuleTriple(const toolchain::Triple &triple) {
  // We unconditionally normalize to "apple" because it's relatively common for
  // build systems to omit the vendor name or use an incorrect one like
  // "unknown". Most parts of the compiler ignore the vendor, so you might not
  // notice such a mistake.
  //
  // Please don't depend on this behavior--specify 'apple' if you're building
  // for an Apple platform.

  assert(triple.isOSDarwin() &&
         "shouldn't normalize non-Darwin triple to 'apple'");

  return "apple";
}

static StringRef
getOSForAppleTargetSpecificModuleTriple(const toolchain::Triple &triple) {
  auto tripleOSName = triple.getOSName();

  // Truncate the OS name before the first digit. "Digit" here is ASCII '0'-'9'.
  auto tripleOSNameNoVersion = tripleOSName.take_until(toolchain::isDigit);

  return toolchain::StringSwitch<StringRef>(tripleOSNameNoVersion)
              .Cases("macos", "macosx", "darwin", "macos")
              .Cases("unknown", "", "unknown")
  // These values are also supported, but are handled by the default case below:
  //          .Case ("ios", "ios")
  //          .Case ("tvos", "tvos")
  //          .Case ("watchos", "watchos")
              .Default(tripleOSNameNoVersion);
}

static std::optional<StringRef>
getEnvironmentForAppleTargetSpecificModuleTriple(const toolchain::Triple &triple) {
  auto tripleEnvironment = triple.getEnvironmentName();
  return toolchain::StringSwitch<std::optional<StringRef>>(tripleEnvironment)
      .Cases("unknown", "", std::nullopt)
      // These values are also supported, but are handled by the default case
      // below:
      //          .Case ("simulator", StringRef("simulator"))
      //          .Case ("macabi", StringRef("macabi"))
      .Default(tripleEnvironment);
}

toolchain::Triple language::getTargetSpecificModuleTriple(const toolchain::Triple &triple) {
  // isOSDarwin() returns true for all Darwin-style OSes, including macOS, iOS,
  // etc.
  if (triple.isOSDarwin()) {
    StringRef newArch = getArchForAppleTargetSpecificModuleTriple(triple);

    StringRef newVendor = getVendorForAppleTargetSpecificModuleTriple(triple);

    StringRef newOS = getOSForAppleTargetSpecificModuleTriple(triple);

    std::optional<StringRef> newEnvironment =
        getEnvironmentForAppleTargetSpecificModuleTriple(triple);

    if (!newEnvironment)
      // Generate an arch-vendor-os triple.
      return toolchain::Triple(newArch, newVendor, newOS);

    // Generate an arch-vendor-os-environment triple.
    return toolchain::Triple(newArch, newVendor, newOS, *newEnvironment);
  }

  // android - drop the API level.  That is not pertinent to the module; the API
  // availability is handled by the clang importer.
  if (triple.isAndroid()) {
    StringRef environment =
        toolchain::Triple::getEnvironmentTypeName(triple.getEnvironment());

    return toolchain::Triple(triple.getArchName(), triple.getVendorName(),
                        triple.getOSName(), environment);
  }

  if (triple.isOSFreeBSD()) {
    return language::getUnversionedTriple(triple);
  }

  if (triple.isOSOpenBSD()) {
    StringRef arch = language::getMajorArchitectureName(triple);
    return toolchain::Triple(arch, triple.getVendorName(), triple.getOSName());
  }

  // Other platforms get no normalization.
  return triple;
}

toolchain::Triple language::getUnversionedTriple(const toolchain::Triple &triple) {
  StringRef unversionedOSName = triple.getOSName().take_until(toolchain::isDigit);
  if (triple.getEnvironment()) {
    StringRef environment =
        toolchain::Triple::getEnvironmentTypeName(triple.getEnvironment());

    return toolchain::Triple(triple.getArchName(), triple.getVendorName(),
                        unversionedOSName, environment);
  }

  return toolchain::Triple(triple.getArchName(), triple.getVendorName(),
                      unversionedOSName);
}

namespace {

// Here, we statically reflect the entire contents of RuntimeVersions.def
// into the template-argument structure of the type AllStaticCodiraReleases.
// We then use template metaprogramming on this type to synthesize arrays
// of PlatformCodiraRelease for each of the target platforms with
// deployment restrictions. This would be much simpler with the recent
// generalizations of constexpr and non-type template parameters, but
// those remain above our baseline for now, so we have to do this the
// old way.

/// A specific release of a platform that provides a specific Codira
/// runtime version. Ultimately, all the variadic goop below is just
/// building an array of these for each platform, which is what we'll
/// use at runtime.
struct PlatformCodiraRelease {
  toolchain::VersionTuple languageVersion;
  toolchain::VersionTuple platformVersion;
};

/// A deployment-restricted platform.
enum class PlatformKind {
  macOS,
  iOS,
  watchOS,
  visionOS
};

/// A template which statically reflects a version tuple. Generalized
/// template parameters would theoretically let us just use
/// toolchain::VersionTuple.
template <unsigned... Components>
struct StaticVersion;

/// A template which statically reflects a single PLATFORM in
/// RuntimeVersions.def.
template <PlatformKind Kind, class Version>
struct StaticPlatformRelease;

/// A template which statically reflects a single RUNTIME_VERSION in
/// RuntimeVersions.def.
template <class CodiraVersion, class PlatformReleases>
struct StaticCodiraRelease;

/// In the assumed context of a particular platform, the release
/// of the platform that first provided a particular Codira version.
template <class CodiraVersion, class PlatformVersion>
struct StaticPlatformCodiraRelease;

// C++ does not allow template argument lists to have trailing commas,
// so to make the macro metaprogramming side of this work, we have to
// include an extra type here (and special-case it in the transforms
// below) for the sole purpose of terminating the list without a comma.
struct Terminal;

#define UNPARENTHESIZE(...) __VA_ARGS__

using AllStaticCodiraReleases =
  packs::Pack<
#define PLATFORM(NAME, VERSION)                                    \
      StaticPlatformRelease<                                       \
        PlatformKind::NAME,                                        \
        StaticVersion<UNPARENTHESIZE VERSION>                      \
      >,
#define FUTURE
#define RUNTIME_VERSION(LANGUAGE_TUPLE, PROVIDERS)                    \
    StaticCodiraRelease<StaticVersion<UNPARENTHESIZE LANGUAGE_TUPLE>,  \
                       packs::Pack<PROVIDERS Terminal>>,
#include "language/AST/RuntimeVersions.def"
    Terminal
  >;

#undef UNPARENTHESIZE

/// A template for comparing two StaticVersion type values.
template <class A, class B>
struct StaticVersionGT;
// 0.0 is not strictly greater than any version.
template <class Second>
struct StaticVersionGT<
    StaticVersion<>,
    Second
  > {
  static constexpr bool value = false;
};
// A version is strictly greater than 0.0 if it has any nonzero component.
template <unsigned FirstHead, unsigned... FirstTail>
struct StaticVersionGT<
    StaticVersion<FirstHead, FirstTail...>,
    StaticVersion<>
  > {
  static constexpr bool value =
    (FirstHead > 0) ? true :
    StaticVersionGT<StaticVersion<FirstTail...>,
                    StaticVersion<>>::value;
};
// a.b is strictly greater than c.d if (a > c || (a == c && b > d)).
template <unsigned FirstHead, unsigned... FirstTail,
          unsigned SecondHead, unsigned... SecondTail>
struct StaticVersionGT<
    StaticVersion<FirstHead, FirstTail...>,
    StaticVersion<SecondHead, SecondTail...>
  > {
  static constexpr bool value =
    (FirstHead > SecondHead) ? true :
    (FirstHead < SecondHead) ? false :
    StaticVersionGT<StaticVersion<FirstTail...>,
                    StaticVersion<SecondTail...>>::value;
};

/// A template for turning a StaticVersion into an toolchain::VersionTuple.
template <class>
struct BuildVersionTuple;

template <unsigned... Components>
struct BuildVersionTuple<StaticVersion<Components...>> {
  static constexpr toolchain::VersionTuple get() {
    return toolchain::VersionTuple(Components...);
  }
};

/// A transform that takes a StaticPlatformRelease, checks if it
/// matches the given platform, and turns it into a
/// StaticPlatformCodiraRelease if so. The result is returned as an
/// optional pack which will be empty if the release is for a different
/// platform.
template <class, class>
struct BuildStaticPlatformCodiraReleaseHelper;
template <PlatformKind Platform, class CodiraVersion>
struct BuildStaticPlatformCodiraReleaseHelper_Arg;

// Matching case.
template <PlatformKind Platform, class CodiraVersion, class PlatformVersion>
struct BuildStaticPlatformCodiraReleaseHelper<
         BuildStaticPlatformCodiraReleaseHelper_Arg<Platform, CodiraVersion>,
         StaticPlatformRelease<Platform, PlatformVersion>> {
  using result = packs::Pack<
    StaticPlatformCodiraRelease<CodiraVersion, PlatformVersion>
  >;
};

// Non-matching case.
template <PlatformKind Platform, class CodiraVersion,
          PlatformKind OtherPlatform, class PlatformVersion>
struct BuildStaticPlatformCodiraReleaseHelper<
         BuildStaticPlatformCodiraReleaseHelper_Arg<Platform, CodiraVersion>,
         StaticPlatformRelease<OtherPlatform, PlatformVersion>> {
  using result = packs::Pack<>;
};

// Terminal case (see above).
template <PlatformKind Platform, class CodiraVersion>
struct BuildStaticPlatformCodiraReleaseHelper<
         BuildStaticPlatformCodiraReleaseHelper_Arg<Platform, CodiraVersion>,
         Terminal> {
  using result = packs::Pack<>;
};


/// A transform that takes a StaticCodiraRelease, finds the platform
/// release in it that matches the given platform, and turns it into
/// StaticPlatformCodiraRelease. The result is returned as an optional
/// pack which will be empty if there is no release for the given
/// platform in this SSR.
template <class, class>
struct BuildStaticPlatformCodiraRelease;
template <PlatformKind Platform>
struct BuildStaticPlatformCodiraRelease_Arg;

// Main case: destructure the arguments, then flat-map our helper
// transform above. Note that we assume that there aren't multiple
// entries for the same platform in the platform releases of a given
// Codira release.
template <PlatformKind Platform, class CodiraVersion,
          class StaticPlatformReleases>
struct BuildStaticPlatformCodiraRelease<
         BuildStaticPlatformCodiraRelease_Arg<Platform>,
         StaticCodiraRelease<CodiraVersion, StaticPlatformReleases>>
  : packs::PackFlatMap<
      BuildStaticPlatformCodiraReleaseHelper,
      BuildStaticPlatformCodiraReleaseHelper_Arg<Platform, CodiraVersion>,
      StaticPlatformReleases> {};

// Terminal case (see above).
template <PlatformKind Platform>
struct BuildStaticPlatformCodiraRelease<
         BuildStaticPlatformCodiraRelease_Arg<Platform>,
         Terminal> {
  using result = packs::Pack<>;
};

/// A template for generating a PlatformCodiraRelease array element
/// from a StaticPlatformCodiraRelease type value.
template <class>
struct BuildPlatformCodiraRelease;

template <class CodiraVersion, class PlatformVersion>
struct BuildPlatformCodiraRelease<
         StaticPlatformCodiraRelease<CodiraVersion, PlatformVersion>
       > {
  static constexpr PlatformCodiraRelease get() {
    return { BuildVersionTuple<CodiraVersion>::get(),
             BuildVersionTuple<PlatformVersion>::get() };
  }
};

/// A template for comparing two StaticPlatformCodiraRelease type values,
/// for the purposes of a well-ordered assertion we want to make:
/// We don't call this GT because it's not really a general-purpose
/// comparison.
template <class, class>
struct StaticPlatformCodiraReleaseStrictlyDescend;

template <class FirstCodira, class FirstPlatform,
          class SecondCodira, class SecondPlatform>
struct StaticPlatformCodiraReleaseStrictlyDescend<
    StaticPlatformCodiraRelease<FirstCodira, FirstPlatform>,
    StaticPlatformCodiraRelease<SecondCodira, SecondPlatform>
  > {
  static constexpr bool value =
    StaticVersionGT<FirstCodira, SecondCodira>::value &&
    StaticVersionGT<FirstPlatform, SecondPlatform>::value;
};

/// A helper template for BuildPlatformCodiraReleaseArray, below.
template <class P>
struct BuildPlatformCodiraReleaseArrayHelper;

template <class... StaticPlatformCodiraReleases>
struct BuildPlatformCodiraReleaseArrayHelper<
         packs::Pack<StaticPlatformCodiraReleases...>
       > {
  // After we reverse the entries, we expect them to strictly
  // descend in both the Codira version and the platform version.
  static_assert(packs::PackComponentsAreOrdered<
                  StaticPlatformCodiraReleaseStrictlyDescend,
                  StaticPlatformCodiraReleases...
                >::value,
                "RuntimeVersions.def is not properly ordered?");
  static constexpr PlatformCodiraRelease releases[] = {
    BuildPlatformCodiraRelease<StaticPlatformCodiraReleases>::get()...
  };
};

/// Build a static constexpr array of PlatformRelease objects matching
/// the given platform.
template <PlatformKind Platform>
struct BuildPlatformCodiraReleaseArray
  : BuildPlatformCodiraReleaseArrayHelper<
      // Turn each entry in AllStaticCodiraReleases into an optional
      // StaticPlatformCodiraRelease representing whether there is a
      // platform release providing that Codira release for the given
      // platform. Flatten that pack, then reverse it so that it's in
      // order of descending release versions. Finally, build an array
      // of PlatformRelease objects for the remaining values.
      typename packs::PackReverse<
        typename packs::PackFlatMap<
          BuildStaticPlatformCodiraRelease,
          BuildStaticPlatformCodiraRelease_Arg<Platform>,
          AllStaticCodiraReleases
        >::result
      >::result
    > {};

} // end anonymous namespace

static std::optional<toolchain::VersionTuple>
findCodiraRuntimeVersionHelper(toolchain::VersionTuple targetPlatformVersion,
                              toolchain::VersionTuple minimumCodiraVersion,
                              ArrayRef<PlatformCodiraRelease> allReleases) {
  #define MAX(a, b) ((a) > (b) ? (a) : (b))

  // Scan forward in our filtered platform release array for the given
  // platform.
  for (auto &release : allReleases) {
    // If the provider version is <= the deployment target, then
    // the deployment target includes support for the given Codira
    // release. Since we're scanning in reverse order of Codira
    // releases (because of the order of entries in RuntimeVersions.def),
    // this must be the highest supported Codira release.
    if (release.platformVersion <= targetPlatformVersion) {
      return std::max(release.codeVersion, minimumCodiraVersion);
    }
  }

  // If we didn't find anything, but the target release is at least the
  // notional future-release version, return that we aren't
  // deployment-limited.
  if (targetPlatformVersion >= toolchain::VersionTuple(99, 99))
    return std::nullopt;

  // Otherwise, return the minimum Codira version.
  return minimumCodiraVersion;
}

/// Return the highest Codira release that matches the given platform and
/// has a version no greater than the target version. Don't return a version
/// older that the minimum. Returns null if the target version matches the
/// notional future release version.
template <PlatformKind TargetPlatform>
static std::optional<toolchain::VersionTuple>
findCodiraRuntimeVersion(toolchain::VersionTuple targetPlatformVersion,
                        toolchain::VersionTuple minimumCodiraVersion) {
  auto &allReleases =
    BuildPlatformCodiraReleaseArray<TargetPlatform>::releases;

  return findCodiraRuntimeVersionHelper(targetPlatformVersion,
                                       minimumCodiraVersion,
                                       allReleases);
}

std::optional<toolchain::VersionTuple>
language::getCodiraRuntimeCompatibilityVersionForTarget(
    const toolchain::Triple &Triple) {

  if (Triple.isMacOSX()) {
    toolchain::VersionTuple OSVersion;
    Triple.getMacOSXVersion(OSVersion);

    // macOS releases predate the stable ABI, so use Codira 5.0 as our base.
    auto baseRelease = toolchain::VersionTuple(5, 0);

    // macOS got its first arm64(e) support in 11.0, which included Codira 5.3.
    if (Triple.isAArch64())
      baseRelease = toolchain::VersionTuple(5, 3);

    return findCodiraRuntimeVersion<PlatformKind::macOS>(OSVersion, baseRelease);

  } else if (Triple.isiOS()) { // includes tvOS
    toolchain::VersionTuple OSVersion = Triple.getiOSVersion();

    // iOS releases predate the stable ABI, so use Codira 5.0 as our base.
    auto baseRelease = toolchain::VersionTuple(5, 0);

    // arm64 simulators and macCatalyst were introduced in iOS 14.0/tvOS 14.0,
    // which included Codira 5.3.
    if (Triple.isAArch64() &&
        (Triple.isSimulatorEnvironment() ||
         Triple.isMacCatalystEnvironment()))
      baseRelease = toolchain::VersionTuple(5, 3);

    // iOS first got arm64e support in 12.0, which did not yet support the
    // Codira stable ABI, so it does not provide a useful version bump.

    return findCodiraRuntimeVersion<PlatformKind::iOS>(OSVersion, baseRelease);

  } else if (Triple.isWatchOS()) {
    toolchain::VersionTuple OSVersion = Triple.getWatchOSVersion();

    // watchOS releases predate the stable ABI, so use Codira 5.0 as our base.
    auto baseRelease = toolchain::VersionTuple(5, 0);

    // 64-bit watchOS was first supported by watchOS 7, which provided
    // Codira 5.3.
    if (Triple.isArch64Bit())
      baseRelease = toolchain::VersionTuple(5, 3);

    return findCodiraRuntimeVersion<PlatformKind::watchOS>(OSVersion, baseRelease);

  } else if (Triple.isXROS()) {
    toolchain::VersionTuple OSVersion = Triple.getOSVersion();

    // visionOS 1.0 provided Codira 5.9.
    auto baseRelease = toolchain::VersionTuple(5, 9);

    return findCodiraRuntimeVersion<PlatformKind::visionOS>(OSVersion, baseRelease);
  }

  return std::nullopt;
}

static const toolchain::VersionTuple minimumMacCatalystDeploymentTarget() {
  return toolchain::VersionTuple(13, 1);
}

toolchain::VersionTuple language::getTargetSDKVersion(clang::DarwinSDKInfo &SDKInfo,
                                              const toolchain::Triple &triple) {
  // Retrieve the SDK version.
  auto SDKVersion = SDKInfo.getVersion();

  // For the Mac Catalyst environment, we have a macOS SDK with a macOS
  // SDK version. Map that to the corresponding iOS version number to pass
  // down to the linker.
  if (tripleIsMacCatalystEnvironment(triple)) {
    if (const auto *MacOStoMacCatalystMapping = SDKInfo.getVersionMapping(
            clang::DarwinSDKInfo::OSEnvPair::macOStoMacCatalystPair())) {
      return MacOStoMacCatalystMapping
          ->map(SDKVersion, minimumMacCatalystDeploymentTarget(), std::nullopt)
          .value_or(toolchain::VersionTuple(0, 0, 0));
    }
    return toolchain::VersionTuple(0, 0, 0);
  }

  return SDKVersion;
}

std::optional<toolchain::Triple>
language::getCanonicalTriple(const toolchain::Triple &triple) {
  toolchain::Triple Result = triple;
  // Non-darwin targets do not require canonicalization.
  if (!triple.isOSDarwin())
    return Result;

  // If the OS versions stay the same, return back the same triple.
  const toolchain::VersionTuple inputOSVersion = triple.getOSVersion();
  const bool isOSVersionInValidRange =
      toolchain::Triple::isValidVersionForOS(triple.getOS(), inputOSVersion);
  const toolchain::VersionTuple canonicalVersion =
      toolchain::Triple::getCanonicalVersionForOS(
          triple.getOS(), triple.getOSVersion(), isOSVersionInValidRange);
  if (canonicalVersion == triple.getOSVersion())
    return Result;

  const std::string inputOSName = triple.getOSName().str();
  const std::string inputOSVersionAsStr = inputOSVersion.getAsString();
  const int platformNameLength =
      inputOSName.size() - inputOSVersionAsStr.size();
  if (!StringRef(inputOSName).ends_with(inputOSVersionAsStr) ||
      (platformNameLength <= 0))
    return std::nullopt;

  toolchain::SmallString<64> buffer(inputOSName.substr(0, platformNameLength));
  buffer.append(canonicalVersion.getAsString());
  Result.setOSName(buffer.str());
  return Result;
}

static std::string getPlistEntry(const toolchain::Twine &Path, StringRef KeyName) {
  auto BufOrErr = toolchain::MemoryBuffer::getFile(Path);
  if (!BufOrErr) {
    // FIXME: diagnose properly
    return {};
  }

  std::string Key = "<key>";
  Key += KeyName;
  Key += "</key>";

  StringRef Lines = BufOrErr.get()->getBuffer();
  while (!Lines.empty()) {
    StringRef CurLine;
    std::tie(CurLine, Lines) = Lines.split('\n');
    if (CurLine.find(Key) != StringRef::npos) {
      std::tie(CurLine, Lines) = Lines.split('\n');
      unsigned Begin = CurLine.find("<string>") + strlen("<string>");
      unsigned End = CurLine.find("</string>");
      return CurLine.substr(Begin, End - Begin).str();
    }
  }

  return {};
}

std::string language::getSDKBuildVersionFromPlist(StringRef Path) {
  return getPlistEntry(Path, "ProductBuildVersion");
}

std::string language::getSDKBuildVersion(StringRef Path) {
  return getSDKBuildVersionFromPlist((toolchain::Twine(Path) +
    "/System/Library/CoreServices/SystemVersion.plist").str());
}

std::string language::getSDKName(StringRef Path) {
  std::string Name = getPlistEntry(toolchain::Twine(Path)+"/SDKSettings.plist",
                                   "CanonicalName");
  if (Name.empty() && Path.ends_with(".sdk")) {
    Name = toolchain::sys::path::filename(Path).drop_back(strlen(".sdk")).str();
  }
  return Name;
}
