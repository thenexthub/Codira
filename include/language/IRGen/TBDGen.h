//===--- TBDGen.h - Public interface to TBDGen ------------------*- C++ -*-===//
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
#ifndef LANGUAGE_IRGEN_TBDGEN_H
#define LANGUAGE_IRGEN_TBDGEN_H

#include "language/Basic/Version.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/StringSet.h"
#include <vector>

namespace toolchain {
class raw_ostream;
}

namespace language {
class FileUnit;
class ModuleDecl;
class TBDGenDescriptor;

/// Options for controlling the exact set of symbols included in the TBD
/// output.
struct TBDGenOptions {
  /// Whether this compilation has multiple IRGen instances.
  bool HasMultipleIGMs = false;

  /// Whether this compilation is producing a TBD for InstallAPI.
  bool IsInstallAPI = false;

  /// Only collect linker directive symbols.
  bool LinkerDirectivesOnly = false;

  /// Whether to include only symbols with public or package linkage.
  bool PublicOrPackageSymbolsOnly = true;

  /// Whether LLVM IR Virtual Function Elimination is enabled.
  bool VirtualFunctionElimination = false;

  /// Whether LLVM IR Witness Method Elimination is enabled.
  bool WitnessMethodElimination = false;

  /// Whether resilient protocols should be emitted fragile.
  bool FragileResilientProtocols = false;

  /// The install_name to use in the TBD file.
  std::string InstallName;

  /// The module link name (for force loading).
  std::string ModuleLinkName;

  /// The current project version to use in the generated TBD file. Defaults
  /// to empty string if not provided.
  std::string CurrentVersion;

  /// The dylib compatibility-version to use in the generated TBD file. Defaults
  /// to empty string if not provided.
  std::string CompatibilityVersion;

  /// The path to a Json file indicating the module name to install-name map
  /// used by @_originallyDefinedIn
  std::string ModuleInstallNameMapPath;

  /// For these modules, TBD gen should embed their symbols in the emitted tbd
  /// file.
  /// Typically, these modules are static linked libraries. Thus their symbols
  /// are embedded in the current dylib.
  std::vector<std::string> embedSymbolsFromModules;

  friend bool operator==(const TBDGenOptions &lhs, const TBDGenOptions &rhs) {
    return lhs.HasMultipleIGMs == rhs.HasMultipleIGMs &&
           lhs.IsInstallAPI == rhs.IsInstallAPI &&
           lhs.LinkerDirectivesOnly == rhs.LinkerDirectivesOnly &&
           lhs.PublicOrPackageSymbolsOnly == rhs.PublicOrPackageSymbolsOnly &&
           lhs.VirtualFunctionElimination == rhs.VirtualFunctionElimination &&
           lhs.WitnessMethodElimination == rhs.WitnessMethodElimination &&
           lhs.FragileResilientProtocols == rhs.FragileResilientProtocols &&
           lhs.InstallName == rhs.InstallName &&
           lhs.ModuleLinkName == rhs.ModuleLinkName &&
           lhs.CurrentVersion == rhs.CurrentVersion &&
           lhs.CompatibilityVersion == rhs.CompatibilityVersion &&
           lhs.ModuleInstallNameMapPath == rhs.ModuleInstallNameMapPath &&
           lhs.embedSymbolsFromModules == rhs.embedSymbolsFromModules;
  }

  friend bool operator!=(const TBDGenOptions &lhs, const TBDGenOptions &rhs) {
    return !(lhs == rhs);
  }

  friend toolchain::hash_code hash_value(const TBDGenOptions &opts) {
    using namespace toolchain;
    return hash_combine(
        opts.HasMultipleIGMs, opts.IsInstallAPI, opts.LinkerDirectivesOnly,
        opts.PublicOrPackageSymbolsOnly, opts.VirtualFunctionElimination,
        opts.WitnessMethodElimination, opts.FragileResilientProtocols,
        opts.InstallName, opts.ModuleLinkName,
        opts.CurrentVersion, opts.CompatibilityVersion,
        opts.ModuleInstallNameMapPath,
        hash_combine_range(opts.embedSymbolsFromModules.begin(),
                           opts.embedSymbolsFromModules.end()));
  }
};

std::vector<std::string> getPublicSymbols(TBDGenDescriptor desc);

void writeTBDFile(ModuleDecl *M, toolchain::raw_ostream &os,
                  const TBDGenOptions &opts);

void writeAPIJSONFile(ModuleDecl *M, toolchain::raw_ostream &os, bool PrettyPrint);

} // end namespace language

#endif
