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

#include "anitextdecoder_fuzzer.h"

#include "test_common.h"
#include "api/ani_textdecoder.h"
#include "core/stdlib_ani_helpers.h"

#include <cstddef>
#include <cstdint>

using TextDecoder = ark::ets::sdk::util::TextDecoder;

namespace OHOS {
class TextDecoderEngine : public FuzzTestEngine {
public:
    static TextDecoderEngine *GetInstance()
    {
        static TextDecoderEngine instanceEngine;
        return &instanceEngine;
    }

    void AniTextDecoder(const char *source, int32_t offset, int32_t length, bool iflag)
    {
        std::string encodingStr = "GBK";
        TextDecoder decoder(encodingStr, iflag);
        decoder.DecodeToString(env_, source, offset, length, iflag);
    }

private:
    TextDecoderEngine() : FuzzTestEngine() {}
};

void AniTextDecoderFuzzTest(const char *data, size_t size)
{
    if (size <= 0) {
        return;
    }

    if (size > MAX_INPUT_SIZE) {
        size = MAX_INPUT_SIZE;
    }

    TextDecoderEngine *engine = TextDecoderEngine::GetInstance();
    engine->AniTextDecoder(data, 0, size, true);
    engine->AniTextDecoder(data, 0, size, false);
    engine->AniTextDecoder(data, 1, size - 1, true);
    engine->AniTextDecoder(data, 1, size - 1, false);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const char *data, size_t size)
{
    /* Run your code on data */
    OHOS::AniTextDecoderFuzzTest(data, size);
    return 0;
}
