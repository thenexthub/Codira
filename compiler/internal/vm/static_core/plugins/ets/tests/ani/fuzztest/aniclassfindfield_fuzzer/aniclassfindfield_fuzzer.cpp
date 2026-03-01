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

#include "ani.h"
#include "test_common.h"
#include "aniclassfindfield_fuzzer.h"

#include <cstddef>
#include <cstdint>
// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)

namespace OHOS {
void AniClassFindFieldFuzzTest(const char *data, size_t size)
{
    if (size <= 0U) {
        return;
    }

    if (size > MAX_INPUT_SIZE) {
        size = MAX_INPUT_SIZE;
    }
    AniFuzzEngine *engine = AniFuzzEngine::GetInstance();
    ani_env *env {};
    engine->GetAniEnv(&env);
    ani_class intCls {};
    env->FindClass("std.core.Int", &intCls);

    ani_field res {};
    std::string input(data, size);
    env->Class_FindField(intCls, input.c_str(), &res);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AniClassFindFieldFuzzTest(reinterpret_cast<const char *>(data), size);
    return 0;
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)
