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

#ifndef _cfunichar_bitmap_data_h
#define _cfunichar_bitmap_data_h

#include "_CStdlib.h"

typedef struct {
    uint32_t _numPlanes;
    uint8_t const * const * const _planes;
} __CFUniCharBitmapData;

#endif /* _cfunichar_bitmap_data_h */
