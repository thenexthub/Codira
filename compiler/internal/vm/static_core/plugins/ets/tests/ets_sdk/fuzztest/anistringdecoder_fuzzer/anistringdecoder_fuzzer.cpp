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

#include "anistringdecoder_fuzzer.h"

#include "test_common.h"
#include "api/ani_stringdecoder.h"
#include "core/stdlib_ani_helpers.h"

#include <cstddef>
#include <cstdint>

using StringDecoder = ark::ets::sdk::util::StringDecoder;

namespace OHOS {
class StringDecoderEngine : public FuzzTestEngine {
public:
    static StringDecoderEngine *GetInstance()
    {
        static StringDecoderEngine instanceEngine;
        return &instanceEngine;
    }

    void AniStringDecoder(const char *source, int32_t offset, int32_t length)
    {
        std::string encodingStr = "utf-8";
        StringDecoder decoder(encodingStr);
        decoder.Write(env_, source, offset, length);
        decoder.End(env_);
    }

    void AniStringDecoderEnd(const char *source, int32_t offset, int32_t length)
    {
        std::string encodingStr = "utf-8";
        StringDecoder decoder(encodingStr);
        decoder.Write(env_, source, offset, length);
        int32_t halfLength = length / 2;
        decoder.End(env_, source, offset, halfLength);
    }

private:
    StringDecoderEngine() : FuzzTestEngine() {}
};

void AniStringDecoderFuzzTest(const char *data, size_t size)
{
    if (size <= 0) {
        return;
    }

    if (size > MAX_INPUT_SIZE) {
        size = MAX_INPUT_SIZE;
    }

    StringDecoderEngine *engine = StringDecoderEngine::GetInstance();
    engine->AniStringDecoder(data, 0, size);
    engine->AniStringDecoder(data, 1, size - 1);
    engine->AniStringDecoderEnd(data, 0, size);
    engine->AniStringDecoderEnd(data, 1, size - 1);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const char *data, size_t size)
{
    /* Run your code on data */
    OHOS::AniStringDecoderFuzzTest(data, size);
    return 0;
}
