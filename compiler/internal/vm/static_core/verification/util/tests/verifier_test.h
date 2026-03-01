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

#ifndef PANDA_VERIFICATION_UTIL_TESTS_VERIFIER_TEST_H
#define PANDA_VERIFICATION_UTIL_TESTS_VERIFIER_TEST_H

#include "include/runtime.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {
class VerifierTest : public testing::Test {
public:
    VerifierTest()
    {
        RuntimeOptions options;
        Logger::InitializeDummyLogging();
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetHeapSizeLimit(64_MB);  // NOLINT(readability-magic-numbers)
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~VerifierTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(VerifierTest);
    NO_MOVE_SEMANTIC(VerifierTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_;
};
}  // namespace ark::verifier::test

#endif  // PANDA_VERIFICATION_UTIL_TESTS_VERIFIER_TEST_H
