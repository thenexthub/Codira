/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_DOMAIN_H
#define PANDA_RUNTIME_COROUTINES_COROUTINE_WORKER_DOMAIN_H

namespace ark {

/// @brief Coroutine domain consists of the coroutine workers that have something in common
enum class CoroutineWorkerDomain {
    /// regular workers that do not support interop, are not "exclusive" and support coroutine migration
    GENERAL = 0,
    /// this domain (or domains) consist of the user-provided worker IDs
    EXACT_ID,
    /// single MAIN worker
    MAIN,
    /**
     * workers that are in the "exclusive" mode:
     * - no incoming coroutines allowed
     * - all child coroutines are scheduled to the same worker
     * - no coroutine migration allowed
     * NOTE(ivagin): rename EA to EXCLUSIVE in core part
     */
    EA,
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_COROUTINE_DOMAIN_H */
