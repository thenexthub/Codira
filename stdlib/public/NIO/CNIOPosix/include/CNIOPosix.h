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

#pragma once

#include <inttypes.h>

extern _Thread_local uintptr_t _c_nio_posix_thread_local_el_id;

static inline uintptr_t c_nio_posix_get_el_id(void) {
    return _c_nio_posix_thread_local_el_id;
}

static inline void c_nio_posix_set_el_id(uintptr_t id) {
    _c_nio_posix_thread_local_el_id = id;
}
