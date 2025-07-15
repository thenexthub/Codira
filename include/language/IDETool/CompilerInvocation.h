//===--- CompilerInvocation.h ---------------------------------------------===//
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

#ifndef LANGUAGE_IDE_COMPILERINVOCATION_H
#define LANGUAGE_IDE_COMPILERINVOCATION_H

#include "language/Frontend/Frontend.h"

namespace language {

class CompilerInvocation;

namespace ide {

bool initCompilerInvocation(
    CompilerInvocation &Invocation, ArrayRef<const char *> OrigArgs,
    FrontendOptions::ActionType Action, DiagnosticEngine &Diags,
    StringRef UnresolvedPrimaryFile,
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FileSystem,
    const std::string &languageExecutablePath,
    const std::string &runtimeResourcePath, time_t sessionTimestamp,
    std::string &Error);

bool initInvocationByClangArguments(ArrayRef<const char *> ArgList,
                                    CompilerInvocation &Invok,
                                    std::string &Error);

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_COMPILERINVOCATION_H
