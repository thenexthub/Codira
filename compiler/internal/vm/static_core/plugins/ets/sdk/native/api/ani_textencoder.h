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

#ifndef ANI_TEXTENCODER_H
#define ANI_TEXTENCODER_H
#include <ani.h>
#include <string>
#include <string_view>
#include <unicode/ucnv.h>
#include "tools/format_logger.h"

namespace ani_helper {
std::u16string Utf8ToUtf16LE(std::string_view u8Str, bool *ok = nullptr);
std::u16string Utf8ToUtf16LE(std::string_view u8Str, size_t resultLengthLimit, size_t *nInputCharsConsumed = nullptr,
                             bool *ok = nullptr);
std::u16string Utf16LEToBE(std::u16string_view wstr);
std::string_view Utf8GetPrefix(std::string_view u8Str, size_t resultSizeBytesLimit,
                               size_t *nInputCharsConsumed = nullptr, bool *ok = nullptr);
}  // namespace ani_helper

namespace ark::ets::sdk::util {
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
struct UConverterWrapper {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    const char *encoding;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    UConverter *converter;

    explicit UConverterWrapper(const char *encodingStr);

    UConverterWrapper(const UConverterWrapper &) = delete;
    UConverterWrapper &operator=(const UConverterWrapper &) = delete;

    ~UConverterWrapper()
    {
        ucnv_close(converter);
    }

    bool IsValid() const
    {
        return converter != nullptr;
    }
};

struct WriteEncodedDataResult {
    size_t nInputCharsConsumed;
    size_t resultSizeBytes;
};

struct ArrayBufferInfos {
    void *data;
    size_t length;
};

struct Uint8ArrayWithBufferInfo {
    ani_object arrayObject;
    ani_ref bufferObject;
    void *bufferData;
    size_t bufferLength;
};

ani_arraybuffer DoEncodeInto(ani_env *env, ani_object object, ani_string stringObj, ani_string aniEncoding);
ani_array DoEncodeIntoUint8Array(ani_env *env, ani_object object, ani_string inputStringObj, ani_string encodingObj,
                                 ani_object destObj);
}  // namespace ark::ets::sdk::util
#endif  // ANI_TEXTENCODER_H
