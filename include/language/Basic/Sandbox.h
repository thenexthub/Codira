//===--- Sandbox.h ----------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_SANDBOX_H
#define LANGUAGE_BASIC_SANDBOX_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Allocator.h"

namespace language {
namespace Sandbox {

/// Applies a sandbox invocation to the given command line (if the platform
/// supports it), and returns the modified command line. On platforms that don't
/// support sandboxing, the command line is returned unmodified.
///
/// - Parameters:
///   - command: The command line to sandbox (including executable as first
///              argument)
///   - strictness: The basic strictness level of the standbox.
///   - writableDirectories: Paths under which writing should be allowed, even
///     if they would otherwise be read-only based on the strictness or paths in
///     `readOnlyDirectories`.
///   - readOnlyDirectories: Paths under which writing should be denied, even if
///     they would have otherwise been allowed by the rules implied by the
///     strictness level.
bool apply(toolchain::SmallVectorImpl<toolchain::StringRef> &command,
           toolchain::BumpPtrAllocator &Alloc);

} // namespace Sandbox
} // namespace language

#endif // LANGUAGE_BASIC_SANDBOX_H
