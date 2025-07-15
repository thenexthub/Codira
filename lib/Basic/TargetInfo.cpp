//===--- TargetInfo.cpp - Target information printing --------------------===//
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

#include "language/Basic/TargetInfo.h"
#include "language/Basic/Version.h"
#include "language/Basic/Platform.h"
#include "language/Basic/StringExtras.h"
#include "language/Frontend/Frontend.h"

#include "clang/Basic/TargetInfo.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

/// Print information about a
static void printCompatibilityLibrary(
    toolchain::VersionTuple runtimeVersion, toolchain::VersionTuple maxVersion,
    StringRef filter, StringRef libraryName, bool forceLoad,
    bool &printedAny, toolchain::raw_ostream &out) {
  if (runtimeVersion > maxVersion)
    return;

  if (printedAny) {
    out << ",";
  }

  out << "\n";
  out << "      {";

  out << "\n        \"libraryName\": \"";
  language::writeEscaped(libraryName, out);
  out << "\",";

  out << "\n        \"filter\": \"";
  language::writeEscaped(filter, out);
  out << "\"";

  if (!forceLoad) {
    out << ",\n        \"forceLoad\": false";
  }

  out << "\n      }";

  printedAny = true;
}

namespace language {
namespace targetinfo {
/// Print information about the selected target in JSON.
void printTargetInfo(const CompilerInvocation &invocation,
                     toolchain::raw_ostream &out) {
  out << "{\n";

  // Compiler version, as produced by --version.
  out << "  \"compilerVersion\": \"";
  writeEscaped(version::getCodiraFullVersion(version::Version::getCurrentLanguageVersion()), out);
  out << "\",\n";

  // Distribution tag, if any.
  StringRef tag = version::getCurrentCompilerTag();
  if (!tag.empty()) {
    out << "  \"languageCompilerTag\": \"";
    writeEscaped(tag, out);
    out << "\",\n";
  }

  // Target triple and target variant triple.
  auto runtimeVersion =
    invocation.getIRGenOptions().AutolinkRuntimeCompatibilityLibraryVersion;
  auto &langOpts = invocation.getLangOptions();
  out << "  \"target\": ";
  printTripleInfo(invocation, langOpts.Target, runtimeVersion, out);
  out << ",\n";

  if (auto &variant = langOpts.TargetVariant) {
    out << "  \"targetVariant\": ";
    printTripleInfo(invocation, *variant, runtimeVersion, out);
    out << ",\n";
  }

  // Various paths.
  auto &searchOpts = invocation.getSearchPathOptions();
  out << "  \"paths\": {\n";

  if (!searchOpts.getSDKPath().empty()) {
    out << "    \"sdkPath\": \"";
    writeEscaped(searchOpts.getSDKPath(), out);
    out << "\",\n";
  }

  auto outputPaths = [&](StringRef name, const std::vector<std::string> &paths){
    out << "    \"" << name << "\": [\n";
    toolchain::interleave(paths, [&out](const std::string &path) {
      out << "      \"";
      writeEscaped(path, out);
      out << "\"";
    }, [&out] {
      out << ",\n";
    });
    out << "\n    ],\n";
  };

  outputPaths("runtimeLibraryPaths", searchOpts.RuntimeLibraryPaths);
  outputPaths("runtimeLibraryImportPaths",
              searchOpts.getRuntimeLibraryImportPaths());

  out << "    \"runtimeResourcePath\": \"";
  writeEscaped(searchOpts.RuntimeResourcePath, out);
  out << "\"\n";

  out << "  }\n";

  out << "}\n";
}

// Print information about the target triple in JSON.
void printTripleInfo(const CompilerInvocation &invocation,
                     const toolchain::Triple &triple,
                     std::optional<toolchain::VersionTuple> runtimeVersion,
                     toolchain::raw_ostream &out) {
  out << "{\n";

  out << "    \"triple\": \"";
  writeEscaped(triple.getTriple(), out);
  out << "\",\n";

  out << "    \"unversionedTriple\": \"";
  writeEscaped(getUnversionedTriple(triple).getTriple(), out);
  out << "\",\n";

  out << "    \"moduleTriple\": \"";
  writeEscaped(getTargetSpecificModuleTriple(triple).getTriple(), out);
  out << "\",\n";

  out << "    \"platform\": \"" << getPlatformNameForTriple(triple) << "\",\n";
  out << "    \"arch\": \"" << language::getMajorArchitectureName(triple)
      << "\",\n";

  clang::DiagnosticsEngine DE{new clang::DiagnosticIDs(),
                              new clang::DiagnosticOptions(),
                              new clang::IgnoringDiagConsumer()};
  std::shared_ptr<clang::TargetOptions> TO =
      std::make_shared<clang::TargetOptions>();
  TO->Triple = triple.str();
  clang::TargetInfo *TI = clang::TargetInfo::CreateTargetInfo(DE, TO);
  out << "    \"pointerWidthInBits\": "
      << TI->getPointerWidth(clang::LangAS::Default) << ",\n";
  out << "    \"pointerWidthInBytes\": "
      << TI->getPointerWidth(clang::LangAS::Default) / TI->getCharWidth()
      << ",\n";

  if (runtimeVersion) {
    out << "    \"languageRuntimeCompatibilityVersion\": \"";
    writeEscaped(runtimeVersion->getAsString(), out);
    out << "\",\n";

    // Compatibility libraries that need to be linked.
    out << "    \"compatibilityLibraries\": [";
    bool printedAnyCompatibilityLibrary = false;
#define BACK_DEPLOYMENT_LIB(Version, Filter, LibraryName, ForceLoad)           \
  printCompatibilityLibrary(*runtimeVersion, toolchain::VersionTuple Version,       \
                            #Filter, LibraryName, ForceLoad,                   \
                            printedAnyCompatibilityLibrary, out);
#include "language/Frontend/BackDeploymentLibs.def"

    if (printedAnyCompatibilityLibrary)
      out << "\n   ";
    out << " ],\n";
  } else {
    out << "    \"compatibilityLibraries\": [ ],\n";
  }

  if (tripleBTCFIByDefaultInOpenBSD(triple)) {
#if LANGUAGE_OPENBSD_BTCFI
     out << "    \"openbsdBTCFIEnabled\": true,\n";
#else
     out << "    \"openbsdBTCFIEnabled\": false,\n";
#endif
  } else {
     out << "    \"openbsdBTCFIEnabled\": false,\n";
  }

  out << "    \"librariesRequireRPath\": "
      << (tripleRequiresRPathForCodiraLibrariesInOS(triple) ? "true" : "false")
      << "\n";

  out << "  }";
}
} // namespace targetinfo
} // namespace language
