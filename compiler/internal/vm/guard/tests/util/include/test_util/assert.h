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

#ifndef GUARD_TESTS_UTIL_ASSERT_H
#define GUARD_TESTS_UTIL_ASSERT_H

#define ASSERT_ARRAY_EQUAL(vec1_, vec2_)                                             \
    do {                                                                             \
        ASSERT_EQ((vec1_).size(), (vec2_).size()) << "vectors have different sizes"; \
        for (size_t i = 0; i < (vec1_).size(); i++) {                                \
            ASSERT_EQ((vec1_)[i], (vec2_)[i]) << "mismatch at index" << i;           \
        }                                                                            \
    } while (false)

#define ASSERT_ABCKIT_WRAPPER_SUCCESS(rc) ASSERT_EQ(rc, AbckitWrapperErrorCode::ERR_SUCCESS)
#define ASSERT_GUARD_SUCCESS(rc) ASSERT_EQ(rc, ark::guard::ErrorCode::ERR_SUCCESS)

#endif  // GUARD_TESTS_UTIL_ASSERT_H
