//===-------------- DependencyScanImpl.h - Codira Compiler -----------------===//
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
// Implementation details of the dependency scanning C API
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_DEPENDENCY_SCAN_JSON_H
#define LANGUAGE_DEPENDENCY_SCAN_JSON_H

#include "language-c/DependencyScan/DependencyScan.h"
#include "toolchain/Support/raw_ostream.h"

namespace language::dependencies {

void writePrescanJSON(toolchain::raw_ostream &out,
                      languagescan_import_set_t importSet);
void writeJSON(toolchain::raw_ostream &out,
               languagescan_dependency_graph_t fullDependencies);
} // namespace language::dependencies

#endif // LANGUAGE_DEPENDENCY_SCAN_JSON_H
