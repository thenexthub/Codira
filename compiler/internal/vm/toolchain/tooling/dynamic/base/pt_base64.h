/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_TOOLING_BASE_PT_BASE64_H
#define ECMASCRIPT_TOOLING_BASE_PT_BASE64_H

#include <cstdint>
#include <string>

namespace panda::ecmascript::tooling {
class PtBase64 {
public:
    PtBase64() = default;
    ~PtBase64() = default;

    static std::size_t constexpr DecodedSize(std::size_t n)
    {
        return n / ENCODED_GROUP_BYTES * UNENCODED_GROUP_BYTES;
    }

    static std::size_t constexpr EncodedSize(std::size_t n)
    {
        return ENCODED_GROUP_BYTES * ((n + 2) / UNENCODED_GROUP_BYTES); // 2: byte filling
    }

    static std::pair<std::size_t, bool> Decode(void *output, const char *input, std::size_t len);
    static size_t Encode(char *output, const void *input, std::size_t len);

private:
   static constexpr uint8_t UNENCODED_GROUP_BYTES = 3;
   static constexpr uint8_t ENCODED_GROUP_BYTES = 4;
   static constexpr uint8_t INVAILD_VALUE = 255;
};
}  // namespace panda::ecmascript::tooling
#endif
