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

#ifndef _bplist_h
#define _bplist_h

typedef struct {
    uint8_t    _unused[5];
    uint8_t     _sortVersion;
    uint8_t    _offsetIntSize;
    uint8_t    _objectRefSize;
    uint64_t    _numObjects;
    uint64_t    _topObject;
    uint64_t    _offsetTableOffset;
} BPlistTrailer;

#endif /* _bplist_h */
