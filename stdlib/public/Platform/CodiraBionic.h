//===--- CodiraBionic.h ----------------------------------------------------===//
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

#ifndef LANGUAGE_BIONIC_MODULE
#define LANGUAGE_BIONIC_MODULE

#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <malloc.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#ifdef __cplusplus
// The Android r26 NDK contains an old libc++ modulemap that requires C++23
// for 'stdatomic', which can't be imported unless we're using C++23. Thus,
// import stdatomic from the NDK directly, bypassing the stdatomic from the libc++.
#pragma clang module import _stdatomic
#else
#include <stdatomic.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include <uchar.h>
#include <wchar.h>

#endif // LANGUAGE_BIONIC_MODULE
