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
#ifndef PANDA_INTRUSIVE_TEST_H
#define PANDA_INTRUSIVE_TEST_H

#if defined(INTRUSIVE_TESTING)
#include "libarkbase/macros.h"
#include <atomic>
#include <cstdint>

namespace ark {
class IntrusiveTest {
protected:
    static std::atomic<uint32_t> id_;

public:
    static DISABLE_THREAD_SANITIZER_INSTRUMENTATION void SetId(uint32_t test_id)
    {
        IntrusiveTest::id_ = test_id;
    }

    static DISABLE_THREAD_SANITIZER_INSTRUMENTATION uint32_t GetId()
    {
        return IntrusiveTest::id_;
    }
};
}  // namespace ark
#endif
#endif  // PANDA_INTRUSIVE_TEST_H
