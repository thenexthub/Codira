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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ARRAY_BUFFER_HELPER
#define PANDA_PLUGINS_ETS_RUNTIME_ARRAY_BUFFER_HELPER

#include <string_view>
#include <cstdint>
#include <variant>

#include "include/mem/panda_string.h"
#include "include/mem/panda_containers.h"

namespace ark::ets {
class EtsEscompatArrayBuffer;
}  // namespace ark::ets

namespace ark::ets::intrinsics::helpers {

template <typename T>
class Err {
private:
    T errorMessage_;

public:
    explicit Err(T msg) : errorMessage_(std::move(msg)) {}
    const T &Message() const
    {
        return errorMessage_;
    }
};
template <typename T>
using Result = std::variant<T, Err<PandaString>>;

namespace base64 {
/**
 * @brief Validates whether the input string is a valid Base64-encoded string.
 *
 * @param input The Base64 string to validate.
 * @return true if valid, false otherwise.
 */
[[nodiscard]] bool ValidateBase64Input(std::string_view input) noexcept;

[[nodiscard]] PandaString Decode(std::string_view encodedData);
[[nodiscard]] PandaString Encode(const PandaVector<uint8_t> &binaryData);

}  // namespace base64

namespace encoding {

[[nodiscard]] Result<bool> ValidateBuffer(const EtsEscompatArrayBuffer *buffer) noexcept;
[[nodiscard]] Result<bool> ValidateIndices(int byteLength, int start, int end);

/**
 * @brief Retrieves the encoding string from an optional encoding pointer.
 * Defaults to "utf8" if the pointer is null.
 */
[[nodiscard]] PandaString GetEncoding(const PandaString *encodingObj) noexcept;

PandaString ConvertUtf8Encoding(const PandaVector<uint8_t> &bytes);

PandaString ConvertUtf16Encoding(const PandaVector<uint8_t> &bytes);

PandaString ConvertBase64Encoding(const PandaVector<uint8_t> &bytes, std::string_view encoding);

PandaString ConvertHexEncoding(const PandaVector<uint8_t> &bytes);

PandaString ConvertLatinEncoding(const PandaVector<uint8_t> &bytes);

Result<PandaVector<uint8_t>> ConvertStringToBytes(const PandaString &input, std::string_view encoding, size_t size);

/// @brief Calculates the number of bytes required to represent the string in the given encoding.
Result<int32_t> CalculateStringBytesLength(std::string_view input, std::string_view encoding);

}  // namespace encoding

}  // namespace ark::ets::intrinsics::helpers

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ARRAY_BUFFER_HELPER
