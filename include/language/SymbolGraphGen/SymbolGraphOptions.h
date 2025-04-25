//===--- SymbolGraphOptions.h - Swift SymbolGraph Options -----------------===//
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

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/TargetParser/Triple.h"

#include "language/AST/AttrKind.h"

#ifndef SWIFT_SYMBOLGRAPHGEN_SYMBOLGRAPHOPTIONS_H
#define SWIFT_SYMBOLGRAPHGEN_SYMBOLGRAPHOPTIONS_H

namespace language {
namespace symbolgraphgen {

struct SymbolGraphOptions {
  /// The directory to output the symbol graph JSON files.
  StringRef OutputDir = {};

  /// The target of the module.
  llvm::Triple Target = {};
  /// Pretty-print the JSON with newlines and indentation.
  bool PrettyPrint = false;

  /// The minimum access level that symbols must have in order to be
  /// included in the graph.
  AccessLevel MinimumAccessLevel = AccessLevel::Public;

  /// Emit members gotten through class inheritance or protocol default
  /// implementations with compound, "SYNTHESIZED" USRs.
  bool EmitSynthesizedMembers = false;
  
  /// Whether to print informational messages when rendering
  /// a symbol graph.
  bool PrintMessages = false;
  
  /// Whether to skip docs for symbols with compound, "SYNTHESIZED" USRs.
  bool SkipInheritedDocs = false;

  /// Whether to skip emitting symbols that are implementations of protocol requirements or
  /// inherited from protocol extensions.
  bool SkipProtocolImplementations = false;

  /// Whether to emit symbols with SPI information.
  bool IncludeSPISymbols = false;

  /// Whether to include documentation for clang nodes or not.
  bool IncludeClangDocs = false;

  /// Whether to emit "swift.extension" symbols for extensions to external types
  /// along with "extensionTo" relationships instead of directly associating
  /// members and conformances with the extended nominal.
  bool EmitExtensionBlockSymbols = false;

  /// Whether to print information for private symbols in system modules.
  /// This should be left as `false` when printing a full-module symbol graph,
  /// but SourceKit should be able to load the information when pulling symbol
  /// information for individual queries.
  bool PrintPrivateSystemSymbols = false;

  /// If this has a value specifies an explicit allow list of reexported module
  /// names that should be included symbol graph.
  std::optional<llvm::ArrayRef<StringRef>> AllowedReexportedModules = {};

  /// If set, a list of availability platforms to restrict (or block) when
  /// rendering symbol graphs.
  std::optional<llvm::DenseSet<StringRef>> AvailabilityPlatforms = {};

  /// Whether `AvailabilityPlatforms` is an allow list or a block list.
  bool AvailabilityIsBlockList = false;
};

} // end namespace symbolgraphgen
} // end namespace language

#endif // SWIFT_SYMBOLGRAPHGEN_SYMBOLGRAPHOPTIONS_H
