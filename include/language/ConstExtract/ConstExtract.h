//===---- ConstExtract.h -- Gather Compile-Time-Known Values ----*- C++ -*-===//
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

#ifndef LANGUAGE_CONST_EXTRACT_H
#define LANGUAGE_CONST_EXTRACT_H

#include "language/AST/ConstTypeInfo.h"
#include "toolchain/ADT/ArrayRef.h"
#include <string>
#include <unordered_set>
#include <vector>

namespace toolchain {
class StringRef;
class raw_fd_ostream;
}

namespace language {
class SourceFile;
class DiagnosticEngine;
class ModuleDecl;
} // namespace language

namespace language {
/// Parse a list of string identifiers from a file at the given path,
/// representing names of protocols.
bool
parseProtocolListFromFile(toolchain::StringRef protocolListFilePath,
                          DiagnosticEngine &diags,
                          std::unordered_set<std::string> &protocols);

/// Gather compile-time-known values of properties in nominal type declarations
/// in this file, of types which conform to one of the protocols listed in
/// \c Protocols
std::vector<ConstValueTypeInfo>
gatherConstValuesForPrimary(const std::unordered_set<std::string> &Protocols,
                            const SourceFile *File);

/// Gather compile-time-known values of properties in nominal type declarations
/// in this module, of types which conform to one of the protocols listed in
/// \c Protocols
std::vector<ConstValueTypeInfo>
gatherConstValuesForModule(const std::unordered_set<std::string> &Protocols,
                           ModuleDecl *Module);

/// Serialize a collection of \c ConstValueInfos to JSON at the
/// provided output stream.
bool writeAsJSONToFile(const std::vector<ConstValueTypeInfo> &ConstValueInfos,
                       toolchain::raw_ostream &OS);
} // namespace language

#endif
