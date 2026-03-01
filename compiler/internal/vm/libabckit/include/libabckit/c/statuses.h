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

#ifndef LIBABCKIT_STATUSES_H
#define LIBABCKIT_STATUSES_H

#ifdef __cplusplus
extern "C" {
#endif

enum AbckitStatus {
    ABCKIT_STATUS_NO_ERROR,
    ABCKIT_STATUS_BAD_ARGUMENT,
    ABCKIT_STATUS_MEMORY_ALLOCATION,
    ABCKIT_STATUS_WRONG_MODE,
    ABCKIT_STATUS_WRONG_TARGET,
    ABCKIT_STATUS_WRONG_LITERAL_TYPE,
    ABCKIT_STATUS_UNSUPPORTED,
    ABCKIT_STATUS_WRONG_CTX,
    ABCKIT_STATUS_INTERNAL_ERROR,
    ABCKIT_STATUS_UNKNOWN_API_VERSION,
};

#ifdef __cplusplus
}
#endif

#endif /* LIBABCKIT_STATUSES_H */
