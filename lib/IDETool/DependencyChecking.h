//===--- DependencyChecking.h ---------------------------------------------===//
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

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/Chrono.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include <optional>

namespace language {
class CompilerInstance;

namespace ide {
/// Cache hash code of the dependencies into \p Map . If \p excludeBufferID is
/// specified, other source files are considered "dependencies", otherwise all
/// source files are considered "current"
void cacheDependencyHashIfNeeded(CompilerInstance &CI,
                                 std::optional<unsigned> excludeBufferID,
                                 toolchain::StringMap<toolchain::hash_code> &Map);

/// Check if any dependent files are modified since \p timestamp. If
/// \p excludeBufferID is specified, other source files are considered
/// "dependencies", otherwise all source files are considered "current".
/// \p Map should be the map populated by \c cacheDependencyHashIfNeeded at the
/// previous dependency checking.
bool areAnyDependentFilesInvalidated(
    CompilerInstance &CI, toolchain::vfs::FileSystem &FS,
    std::optional<unsigned> excludeBufferID, toolchain::sys::TimePoint<> timestamp,
    const toolchain::StringMap<toolchain::hash_code> &Map);

} // namespace ide
} // namespace language
