//===----------------------------------------------------------------------===//
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

#ifndef LLVM_SOURCEKITDINPROC_INTERNAL_H
#define LLVM_SOURCEKITDINPROC_INTERNAL_H

#include <string>

namespace sourcekitdInProc {
std::string getRuntimeLibPath();
std::string getSwiftExecutablePath();
std::string getDiagnosticDocumentationPath();
} // namespace sourcekitdInProc

#endif
