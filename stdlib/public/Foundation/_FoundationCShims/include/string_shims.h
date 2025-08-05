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

#ifndef CSHIMS_STRING_H
#define CSHIMS_STRING_H

#include "_CShimsMacros.h"
#include "_CStdlib.h"

#if __has_include(<locale.h>)
#include <locale.h>
#endif
#include <stddef.h>

#if __has_include(<xlocale.h>)
#include <xlocale.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define locale_t _locale_t
#endif

#if defined(TARGET_OS_EXCLAVEKIT) && TARGET_OS_EXCLAVEKIT
#define locale_t void *
#endif

INTERNAL int _stringshims_strncasecmp_clocale(const char * _Nullable s1, const char * _Nullable s2, size_t n);

INTERNAL double _stringshims_strtod_clocale(const char * _Nullable __restrict nptr, char * _Nullable * _Nullable __restrict endptr);

INTERNAL float _stringshims_strtof_clocale(const char * _Nullable __restrict nptr, char * _Nullable * _Nullable __restrict endptr);

#define _STRINGSHIMS_MACROMAN_MAP_SIZE 129
INTERNAL const uint8_t _stringshims_macroman_mapping[_STRINGSHIMS_MACROMAN_MAP_SIZE][3];

#define _STRINGSHIMS_NEXTSTEP_MAP_SIZE 128
INTERNAL const uint16_t _stringshims_nextstep_mapping[_STRINGSHIMS_NEXTSTEP_MAP_SIZE];

#ifdef __cplusplus
}
#endif

#endif /* CSHIMS_STRING_H */
