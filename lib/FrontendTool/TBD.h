//===--- TBD.h -- generates and validates TBD files -------------*- C++ -*-===//
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

#ifndef LANGUAGE_FRONTENDTOOL_TBD_H
#define LANGUAGE_FRONTENDTOOL_TBD_H

#include "language/Frontend/FrontendOptions.h"

namespace toolchain {
class StringRef;
class Module;
namespace vfs {
class OutputBackend;
}
}
namespace language {
class ModuleDecl;
class FileUnit;
class FrontendOptions;
struct TBDGenOptions;

bool writeTBD(ModuleDecl *M, StringRef OutputFilename,
              toolchain::vfs::OutputBackend &Backend, const TBDGenOptions &Opts);
bool validateTBD(ModuleDecl *M,
                 const toolchain::Module &IRModule,
                 const TBDGenOptions &opts,
                 bool diagnoseExtraSymbolsInTBD);
bool validateTBD(FileUnit *M,
                 const toolchain::Module &IRModule,
                 const TBDGenOptions &opts,
                 bool diagnoseExtraSymbolsInTBD);
}

#endif
