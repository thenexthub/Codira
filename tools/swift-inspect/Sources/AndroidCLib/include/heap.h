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

#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Location of the heap_iterate callback.
void* heap_iterate_callback_start();

// Size of the heap_iterate callback.
size_t heap_iterate_callback_len();

// Initialize the provided buffer to receive heap iteration metadata.
bool heap_iterate_metadata_init(void* data, size_t len);

// Callback invoked by heap_iterate_data_process for each heap entry .
typedef void (*heap_iterate_entry_callback_t)(void* context, uint64_t base, uint64_t len);

// Process all heap iteration entries in the provided buffer.
bool heap_iterate_metadata_process(
    void* data, size_t len, void* callback_context, heap_iterate_entry_callback_t callback);

#if defined(__cplusplus)
}
#endif
