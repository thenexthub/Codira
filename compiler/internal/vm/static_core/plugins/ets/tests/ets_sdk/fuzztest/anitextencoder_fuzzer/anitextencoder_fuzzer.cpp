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

#include "anitextencoder_fuzzer.h"

#include "test_common.h"
#include "api/ani_textencoder.h"
#include "core/stdlib_ani_helpers.h"

#include <cstddef>
#include <cstdint>

namespace OHOS {
class TextEncoderEngine : public FuzzTestEngine {
public:
    static TextEncoderEngine *GetInstance()
    {
        static TextEncoderEngine instanceEngine;
        return &instanceEngine;
    }

    void AniTextEncoder(const char *source, int32_t length)
    {
        std::string_view inputString(source, static_cast<size_t>(length));
        std::u16string u16Str = ani_helper::Utf8ToUtf16LE(inputString);
        ani_helper::Utf16LEToBE(u16Str);
    }

private:
    TextEncoderEngine() : FuzzTestEngine() {}
};

void AniTextEncoderFuzzTest(const char *data, size_t size)
{
    if (size <= 0) {
        return;
    }

    if (size > MAX_INPUT_SIZE) {
        size = MAX_INPUT_SIZE;
    }

    TextEncoderEngine *engine = TextEncoderEngine::GetInstance();
    engine->AniTextEncoder(data, size);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const char *data, size_t size)
{
    /* Run your code on data */
    OHOS::AniTextEncoderFuzzTest(data, size);
    return 0;
}
