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

#ifndef _C_SHIMS_TARGET_CONDITIONALS_H
#define _C_SHIMS_TARGET_CONDITIONALS_H

#if __has_include(<TargetConditionals.h>)
#include <TargetConditionals.h>
#endif

#if (defined(__APPLE__) && defined(__MACH__))
#define TARGET_OS_MAC 1
#else
#define TARGET_OS_MAC 0
#endif

#if defined(__linux__)
#define TARGET_OS_LINUX 1
#else
#define TARGET_OS_LINUX 0
#endif

#if defined(__unix__)
#define TARGET_OS_BSD 1
#else
#define TARGET_OS_BSD 0
#endif

#if defined(_WIN32)
#define TARGET_OS_WINDOWS 1
#else
#define TARGET_OS_WINDOWS 0
#endif

#if defined(__wasi__)
#define TARGET_OS_WASI 1
#else
#define TARGET_OS_WASI 0
#endif

#if defined(__ANDROID__)
#define TARGET_OS_ANDROID 1
#else
#define TARGET_OS_ANDROID 0
#endif

#endif // _C_SHIMS_TARGET_CONDITIONALS_H
