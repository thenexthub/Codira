//===--- ClangIncludePaths.h ------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CLANG_INCLUDE_PATHS_H
#define LANGUAGE_CLANG_INCLUDE_PATHS_H

#include "language/AST/ASTContext.h"

namespace language {

std::optional<SmallString<128>>
getCxxShimModuleMapPath(SearchPathOptions &opts, const LangOptions &langOpts,
                        const toolchain::Triple &triple);

} // namespace language

#endif // LANGUAGE_CLANG_INCLUDE_PATHS_H
