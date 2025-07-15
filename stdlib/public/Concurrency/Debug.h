//===--- Debug.h - Codira Concurrency debug helpers --------------*- C++ -*-===//
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
//
// Debugging and inspection support.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CONCURRENCY_DEBUG_H
#define LANGUAGE_CONCURRENCY_DEBUG_H

#include <cstdint>

#include "language/Runtime/Config.h"

namespace language {

// Dispatch knows about these symbol names. Don't change them without consulting
// dispatch.

/// The metadata pointer used for job objects.
LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_jobMetadata;

/// The metadata pointer used for async task objects.
LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_asyncTaskMetadata;

/// The size of an AsyncTask, in bytes.
LANGUAGE_EXPORT_FROM(language_Concurrency)
const size_t _language_concurrency_debug_asyncTaskSize;

/// A fake metadata pointer placed at the start of async task slab allocations.
LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_asyncTaskSlabMetadata;

LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_non_future_adapter;
LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_future_adapter;
LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_task_wait_throwing_resume_adapter;
LANGUAGE_EXPORT_FROM(language_Concurrency)
const void *const _language_concurrency_debug_task_future_wait_resume_adapter;

/// Whether the runtime we are inspecting supports priority escalation
LANGUAGE_EXPORT_FROM(language_Concurrency)
bool _language_concurrency_debug_supportsPriorityEscalation;

/// The current version of internal data structures that lldb may decode.
/// The version numbers used so far are:
/// 1 - The initial version number when this variable was added, in language 6.1.
LANGUAGE_EXPORT_FROM(language_Concurrency)
uint32_t _language_concurrency_debug_internal_layout_version;

} // namespace language

#endif
