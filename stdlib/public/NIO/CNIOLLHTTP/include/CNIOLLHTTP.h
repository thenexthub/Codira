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

// adaptions for llhttp to make it more straightforward to use from Swift

#ifndef C_NIO_LLHTTP_SWIFT
#define C_NIO_LLHTTP_SWIFT

#include "c_nio_llhttp.h"

static inline llhttp_errno_t c_nio_llhttp_execute_swift(llhttp_t *parser,
                                                        const void *data,
                                                        size_t len) {
    return c_nio_llhttp_execute(parser, (const char *)data, len);
}

#endif
