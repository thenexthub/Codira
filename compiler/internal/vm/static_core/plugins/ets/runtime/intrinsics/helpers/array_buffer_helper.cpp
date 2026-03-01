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

#include "array_buffer_helper.h"

#include <array>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <cctype>

#include "plugins/ets/runtime/types/ets_arraybuffer.h"

namespace ark::ets::intrinsics::helpers {

namespace base64 {

constexpr std::string_view K_BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Lookup table size for all possible byte values.
constexpr size_t K_LOOKUP_TABLE_SIZE = 256;

// Base64 block and binary block size constants.
constexpr size_t K_BASE64_BLOCK_SIZE = 4;  // Size of a Base64 encoded block.
constexpr size_t K_MAX_PADDING_CHARS = 2;  // Maximum number of padding characters allowed.
constexpr size_t K_BINARY_BLOCK_SIZE = 3;  // Size of binary data block that becomes one Base64 block.

// Bit manipulation constants.
constexpr size_t K_BITS_PER_BYTE = 8;
constexpr uint32_t K_BASE64_MASK = 0x3F;  // Mask for 6 bits (Base64 character).

// Bit shift constants for Base64 encoding/decoding.
constexpr size_t K_HIGH_BYTE_SHIFT = 16;
constexpr size_t K_MID_BYTE_SHIFT = 12;
constexpr size_t K_LOW_BYTE_SHIFT = 6;
constexpr size_t K_LAST_CHAR_SHIFT = 18;
constexpr size_t K_NUMBER_0 = 0;
constexpr size_t K_NUMBER_1 = 1;
constexpr size_t K_NUMBER_2 = 2;
constexpr size_t K_NUMBER_3 = 3;
constexpr size_t K_NUMBER_4 = 4;

constexpr uint32_t LOWER_8_BITS_MASK = 0x00FFU;
constexpr uint8_t LOWER_6_BITS_MASK = 0x3FU;
constexpr uint8_t LOWER_4_BITS_MASK = 0x0FU;
constexpr uint8_t LOWER_2_BITS_MASK = 0x03U;
constexpr uint8_t MIDDLE_4_BITS_MASK = 0x3CU;
constexpr uint8_t UPPER_2_BITS_MASK = 0x30U;
constexpr uint8_t BITSHIFT_2 = 2U;
constexpr uint8_t BITSHIFT_4 = 4U;
constexpr uint8_t BITSHIFT_6 = 6U;

constexpr char K_PADDING_CHAR = '=';

constexpr std::array<int, K_LOOKUP_TABLE_SIZE> BuildDecodingTable() noexcept
{
    std::array<int, K_LOOKUP_TABLE_SIZE> table {};
    for (auto &entry : table) {
        entry = -1;  // mark as invalid
    }
    for (size_t i = 0; i < K_BASE64_CHARS.size(); ++i) {
        table[static_cast<unsigned char>(K_BASE64_CHARS[i])] = static_cast<int>(i);
    }
    return table;
}

constexpr auto K_DECODING_TABLE = BuildDecodingTable();

[[nodiscard]] constexpr bool IsBase64Character(unsigned char c) noexcept
{
    return K_DECODING_TABLE[c] != -1;
}

[[nodiscard]] bool ValidateBase64Input(std::string_view input) noexcept
{
    if (input.empty()) {
        return true;
    }
    if (input.size() % K_BASE64_BLOCK_SIZE != 0) {
        return false;
    }

    const auto paddingStart = std::find(input.begin(), input.end(), K_PADDING_CHAR);
    const bool hasPadding = paddingStart != input.end();
    // Count padding characters at the end
    const auto paddingCount = static_cast<size_t>(std::count(paddingStart, input.end(), K_PADDING_CHAR));
    const bool validChars =
        std::all_of(input.begin(), paddingStart, [](unsigned char c) { return IsBase64Character(c); });
    const bool validPadding =
        !hasPadding || (paddingCount <= K_MAX_PADDING_CHARS &&
                        std::all_of(paddingStart, input.end(), [](char c) { return c == K_PADDING_CHAR; }));
    return validChars && validPadding;
}

static bool IsBase64Char(uint8_t c)
{
    return ((isalnum(c) != 0) || (c == '+') || (c == '/') || (c == '-') || (c == '_'));
}

void ProcessDecodedBlock(const std::array<uint8_t, K_NUMBER_4> &charArray4, PandaString &decoded)
{
    std::array<uint8_t, K_NUMBER_3> charArray3 = {0};
    charArray3[K_NUMBER_0] =
        static_cast<uint8_t>((static_cast<uint32_t>(charArray4[K_NUMBER_0]) << BITSHIFT_2) +
                             ((static_cast<uint32_t>(charArray4[K_NUMBER_1]) & UPPER_2_BITS_MASK) >> BITSHIFT_4));
    charArray3[K_NUMBER_1] =
        static_cast<uint8_t>(((static_cast<uint32_t>(charArray4[K_NUMBER_1]) & LOWER_4_BITS_MASK) << BITSHIFT_4) +
                             ((static_cast<uint32_t>(charArray4[K_NUMBER_2]) & MIDDLE_4_BITS_MASK) >> BITSHIFT_2));
    charArray3[K_NUMBER_2] = static_cast<uint8_t>(
        ((static_cast<uint32_t>(charArray4[K_NUMBER_2]) & LOWER_2_BITS_MASK) << BITSHIFT_6) + charArray4[K_NUMBER_3]);
    for (uint8_t byte : charArray3) {
        decoded += byte;
    }
}

void ProcessRemainingChars(uint32_t index, std::array<uint8_t, K_NUMBER_4> &charArray4, PandaString &decoded)
{
    std::array<uint8_t, K_NUMBER_3> charArray3 = {0};
    charArray3[K_NUMBER_0] =
        static_cast<uint8_t>((static_cast<uint32_t>(charArray4[K_NUMBER_0]) << BITSHIFT_2) +
                             ((static_cast<uint32_t>(charArray4[K_NUMBER_1]) & UPPER_2_BITS_MASK) >> BITSHIFT_4));
    if (index > K_NUMBER_1) {
        charArray3[K_NUMBER_1] =
            static_cast<uint8_t>(((static_cast<uint32_t>(charArray4[K_NUMBER_1]) & LOWER_4_BITS_MASK) << BITSHIFT_4) +
                                 ((static_cast<uint32_t>(charArray4[K_NUMBER_2]) & LOWER_6_BITS_MASK) >> BITSHIFT_2));
    }

    for (uint32_t i = 0; i < index - 1; i++) {
        decoded += charArray3[i];
    }
}

void DecodeBlock(std::array<uint8_t, K_NUMBER_4> &charArray4)
{
    for (uint32_t i = K_NUMBER_0; i < K_NUMBER_4; i++) {
        charArray4[i] = static_cast<uint32_t>(K_DECODING_TABLE[charArray4[i]]) & LOWER_8_BITS_MASK;
    }
}

[[nodiscard]] PandaString Decode(std::string_view encodedData)
{
    if (encodedData.empty()) {
        return "";
    }
    uint32_t index = 0;
    uint32_t cursor = 0;
    const size_t len = encodedData.size();
    std::array<uint8_t, K_NUMBER_4> charArray4 = {0};
    PandaString decoded;
    while (cursor < len && IsBase64Char(encodedData[cursor])) {
        charArray4[index++] = encodedData[cursor++];
        if (index == K_NUMBER_4) {
            DecodeBlock(charArray4);
            ProcessDecodedBlock(charArray4, decoded);
            index = K_NUMBER_0;
        }
    }
    if (index > K_NUMBER_0) {
        DecodeBlock(charArray4);
        ProcessRemainingChars(index, charArray4, decoded);
    }
    return decoded;
}

[[nodiscard]] PandaString Encode(const PandaVector<uint8_t> &binaryData)
{
    if (binaryData.empty()) {
        return {};
    }
    PandaString encoded;
    const size_t kReserveMultiplier = 2;

    encoded.reserve(((binaryData.size() + kReserveMultiplier) / K_BINARY_BLOCK_SIZE) * K_BASE64_BLOCK_SIZE);

    const size_t kOneByte = 1;
    const size_t kTwoBytes = 2;

    size_t pos = 0;
    while (pos < binaryData.size()) {
        size_t remain = binaryData.size() - pos;
        uint32_t triple = (static_cast<uint32_t>(binaryData[pos]) << K_HIGH_BYTE_SHIFT);
        if (remain > kOneByte) {
            triple |= (static_cast<uint32_t>(binaryData[pos + kOneByte]) << K_BITS_PER_BYTE);
        }
        if (remain > kTwoBytes) {
            triple |= static_cast<uint32_t>(binaryData[pos + kTwoBytes]);
        }

        encoded.push_back(K_BASE64_CHARS[(triple >> K_LAST_CHAR_SHIFT) & K_BASE64_MASK]);
        encoded.push_back(K_BASE64_CHARS[(triple >> K_MID_BYTE_SHIFT) & K_BASE64_MASK]);
        encoded.push_back(remain > kOneByte ? K_BASE64_CHARS[(triple >> K_LOW_BYTE_SHIFT) & K_BASE64_MASK]
                                            : K_PADDING_CHAR);
        encoded.push_back(remain > kTwoBytes ? K_BASE64_CHARS[triple & K_BASE64_MASK] : K_PADDING_CHAR);

        pos += K_BINARY_BLOCK_SIZE;
    }
    return encoded;
}

}  // namespace base64

namespace encoding {

using namespace std::literals::string_view_literals;
// UTF-16 related constants.
constexpr size_t K_UTF16_BYTES_PER_CHAR = 2;  // Number of bytes per UTF-16 character.
constexpr size_t K_HIGH_BYTE_SHIFT = 8;       // Shift for high byte in UTF-16.

constexpr size_t K_HEX_PAIR_SIZE = 2;

// Named constants for bit masks
constexpr uint8_t K_ASCII_MASK = 0x7F;
constexpr uint8_t K_BYTE_MASK = 0xFF;

constexpr uint8_t HIGER_4_BITS_MASK = 0xF0U;
constexpr uint8_t FOUR_BYTES_STYLE = 0xF0U;
constexpr uint8_t THREE_BYTES_STYLE = 0xE0U;
constexpr uint8_t TWO_BYTES_STYLE1 = 0xD0U;
constexpr uint8_t TWO_BYTES_STYLE2 = 0xC0U;
constexpr uint8_t LOWER_6_BITS_MASK = 0x3FU;
constexpr uint8_t LOWER_5_BITS_MASK = 0x1FU;
constexpr uint8_t LOWER_4_BITS_MASK = 0x0FU;
constexpr uint8_t LOWER_3_BITS_MASK = 0x07U;
constexpr uint32_t HIGH_SURROGATE_SHIFT = 10U;
constexpr uint32_t HIGH_AGENT_MASK = 0xD800U;
constexpr uint32_t LOW_AGENT_MASK = 0xDC00U;
constexpr uint32_t UTF8_VALID_BITS = 6U;
constexpr uint32_t UTF8_BITS = 8U;
constexpr uint32_t UTF8_BITS_MASK = 0xFFU;
constexpr uint32_t UTF16_SPECIAL_VALUE = 0x10000U;

constexpr size_t HEX_BASE = 16;
constexpr uint8_t ONE_BYTE_MASK = 0x80U;

constexpr std::array K_SINGLE_BYTE_ENCODINGS = {"utf8"sv, "utf-8"sv, "ascii"sv, "latin1"sv, "binary"sv};
constexpr std::array K_DOUBLE_BYTE_ENCODINGS = {"utf16le"sv, "ucs2"sv, "ucs-2"sv};

constexpr std::array UTF8_ENCODINGS = {"utf8"sv, "utf-8"sv};                // UTF-8 variants
constexpr std::array ASCII_ENCODINGS = {"ascii"sv};                         // ASCII (7-bit)
constexpr std::array UTF16_ENCODINGS = {"utf16le"sv, "ucs2"sv, "ucs-2"sv};  // UTF-16 little-endian variants
constexpr std::array BASE64_ENCODINGS = {"base64"sv, "base64url"sv};        // Base64 variants
constexpr std::array LATIN_ENCODINGS = {"latin1"sv, "binary"sv};            // Latin1/binary encodings

[[nodiscard]] Result<bool> ValidateBuffer(const EtsEscompatArrayBuffer *buffer) noexcept
{
    if (buffer == nullptr) {
        return Err<PandaString>(PandaString("Buffer is null"));
    }
    if (buffer->WasDetached()) {
        return Err<PandaString>(PandaString("Buffer was detached"));
    }
    return true;
}
[[nodiscard]] Result<bool> ValidateIndices(int byteLength, int start, int end)
{
    if (start < 0 || start > byteLength) {
        return Err<PandaString>(PandaString("Start index is out of bounds"));
    }
    if (end < 0 || end > byteLength || end < start) {
        return Err<PandaString>(PandaString("End index is out of bounds"));
    }
    return true;
}
[[nodiscard]] PandaString GetEncoding(const PandaString *encodingObj) noexcept
{
    return encodingObj != nullptr ? *encodingObj : PandaString("utf8");
}
PandaVector<uint8_t> BytesFromString(std::string_view input)
{
    PandaVector<uint8_t> bytes;
    bytes.assign(input.begin(), input.end());
    return bytes;
}

PandaString StringFromBytes(const PandaVector<uint8_t> &bytes)
{
    return PandaString(bytes.begin(), bytes.end());
}

PandaVector<uint8_t> MaskBytes(std::string_view input, uint8_t mask)
{
    PandaVector<uint8_t> bytes;
    bytes.resize(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        bytes[i] = static_cast<uint8_t>(static_cast<unsigned char>(input[i]) & mask);
    }
    return bytes;
}

PandaString ConvertUtf8Encoding(const PandaVector<uint8_t> &bytes)
{
    return StringFromBytes(bytes);
}

PandaString ConvertUtf16Encoding(const PandaVector<uint8_t> &bytes)
{
    PandaString output;
    for (size_t i = 0; i < bytes.size(); i += K_UTF16_BYTES_PER_CHAR) {
        uint16_t ch = static_cast<uint16_t>(bytes[i]) |
                      static_cast<uint16_t>(static_cast<uint16_t>(bytes[i + 1]) << K_HIGH_BYTE_SHIFT);
        output.push_back(static_cast<char>(ch));
    }
    return output;
}

PandaString ConvertBase64Encoding(const PandaVector<uint8_t> &bytes, std::string_view encoding)
{
    PandaString output = base64::Encode(bytes);
    if (encoding == "base64url") {
        std::replace(output.begin(), output.end(), '+', '-');
        std::replace(output.begin(), output.end(), '/', '_');
        size_t lastValidCharPos = output.find_last_not_of('=');
        if (lastValidCharPos != PandaString::npos) {
            output.erase(lastValidCharPos + 1);
        }
    }
    return output;
}

PandaVector<uint8_t> ConvertUtf8ToBytes(std::string_view input)
{
    return BytesFromString(input);
}

PandaVector<uint8_t> ConvertAsciiToBytes(std::string_view input)
{
    return MaskBytes(input, K_ASCII_MASK);
}

PandaVector<uint8_t> ConvertLatinToBytes(std::string_view input)
{
    return MaskBytes(input, K_BYTE_MASK);
}

PandaString ConvertHexEncoding(const PandaVector<uint8_t> &bytes)
{
    PandaOStringStream oss;
    const int width = 2;
    for (uint8_t byte : bytes) {
        oss << std::hex << std::setw(width) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

PandaString ConvertLatinEncoding(const PandaVector<uint8_t> &bytes)
{
    PandaString output;
    output.reserve(bytes.size());
    for (uint8_t byte : bytes) {
        output.push_back(static_cast<char>(byte));
    }
    return output;
}

static bool IsOneByte(uint8_t u8Char)
{
    return (u8Char & ONE_BYTE_MASK) == 0;
}

static void Utf16ToUTF8Bytes(PandaVector<uint8_t> &bytes, uint32_t codePoint)
{
    if (codePoint >= UTF16_SPECIAL_VALUE) {
        codePoint -= UTF16_SPECIAL_VALUE;
        // 10 : a half of 20 , shift right 10 bits
        uint16_t highSurrogate = ((codePoint >> HIGH_SURROGATE_SHIFT) | HIGH_AGENT_MASK);
        uint16_t lowSurrogate = ((codePoint & LOW_AGENT_MASK) | LOW_AGENT_MASK);

        bytes.push_back(highSurrogate & UTF8_BITS_MASK);
        bytes.push_back(static_cast<uint32_t>(highSurrogate >> UTF8_BITS) & UTF8_BITS_MASK);
        bytes.push_back(lowSurrogate & UTF8_BITS_MASK);
        bytes.push_back(static_cast<uint32_t>(lowSurrogate >> UTF8_BITS) & UTF8_BITS_MASK);
    } else {
        bytes.push_back(codePoint & UTF8_BITS_MASK);
        bytes.push_back((codePoint >> UTF8_BITS) & UTF8_BITS_MASK);
    }
}

static bool HasEnoughBytes(size_t currentIndex, size_t requiredLength, size_t inputSize)
{
    return (currentIndex + requiredLength) <= inputSize;
}

static bool ProcessOneByte(unsigned char firstByte, uint32_t &outCodePoint, size_t &outUtf8Length)
{
    if (IsOneByte(static_cast<uint8_t>(firstByte))) {
        outCodePoint = firstByte;
        outUtf8Length = 1;
        return true;
    }
    return false;
}

static bool ProcessTwoBytes(std::string_view input, size_t i, uint32_t &outCodePoint, size_t &outUtf8Length)
{
    auto firstByte = static_cast<unsigned char>(input[i]);
    if (((firstByte & HIGER_4_BITS_MASK) == TWO_BYTES_STYLE1) ||
        ((firstByte & HIGER_4_BITS_MASK) == TWO_BYTES_STYLE2)) {
        if (!HasEnoughBytes(i, 1, input.size())) {
            return false;
        }
        outCodePoint = static_cast<uint32_t>(static_cast<uint8_t>(firstByte & LOWER_5_BITS_MASK) << UTF8_VALID_BITS) |
                       static_cast<uint32_t>((static_cast<unsigned char>(input[i + 1]) & LOWER_6_BITS_MASK));
        // 2: means the length is two
        outUtf8Length = 2;
        return true;
    }
    return false;
}

static bool ProcessThreeBytes(std::string_view input, size_t i, uint32_t &outCodePoint, size_t &outUtf8Length)
{
    auto firstByte = static_cast<unsigned char>(input[i]);
    if ((firstByte & HIGER_4_BITS_MASK) == THREE_BYTES_STYLE) {
        // 2: means two byte after
        if (!HasEnoughBytes(i, 2, input.size())) {
            return false;
        }
        outCodePoint =
            // 2 : shift left 2 times of UTF8_VALID_BITS
            static_cast<uint32_t>(static_cast<uint8_t>(firstByte & LOWER_4_BITS_MASK) << (2 * UTF8_VALID_BITS)) |
            static_cast<uint32_t>(static_cast<uint8_t>((static_cast<unsigned char>(input[i + 1]) & LOWER_6_BITS_MASK))
                                  << UTF8_VALID_BITS) |
            // 2: means the third byte
            static_cast<uint32_t>((static_cast<unsigned char>(input[i + 2]) & LOWER_6_BITS_MASK));
        // 3: means the length is three
        outUtf8Length = 3;
        return true;
    }
    return false;
}

static bool ProcessFourBytes(std::string_view input, size_t i, uint32_t &outCodePoint, size_t &outUtf8Length)
{
    auto firstByte = static_cast<unsigned char>(input[i]);
    if ((firstByte & HIGER_4_BITS_MASK) == FOUR_BYTES_STYLE) {
        // 3: means three byte after
        if (!HasEnoughBytes(i, 3, input.size())) {
            return false;
        }
        outCodePoint =
            // 3 : shift left 3 times of UTF8_VALID_BITS
            static_cast<uint32_t>(static_cast<uint8_t>(firstByte & LOWER_3_BITS_MASK) << (3 * UTF8_VALID_BITS)) |
            static_cast<uint32_t>(static_cast<uint8_t>(static_cast<unsigned char>(input[i + 1]) & LOWER_6_BITS_MASK)
                                  // 2 : shift left 2 times of UTF8_VALID_BITS
                                  << (2 * UTF8_VALID_BITS)) |
            // 2: means the third byte
            static_cast<uint32_t>(static_cast<uint8_t>(static_cast<unsigned char>(input[i + 2]) & LOWER_6_BITS_MASK)
                                  << UTF8_VALID_BITS) |
            // 3: means the forth byte
            static_cast<uint32_t>(static_cast<unsigned char>(input[i + 3]) & LOWER_6_BITS_MASK);
        // 4: means the length is four
        outUtf8Length = 4;
        return true;
    }
    return false;
}

PandaVector<uint8_t> ConvertUtf16ToBytes(std::string_view input)
{
    PandaVector<uint8_t> bytes;
    for (size_t i = 0; i < input.size();) {
        uint32_t codePoint = 0;
        size_t utf8Length = 0;
        auto firstByte = static_cast<unsigned char>(input[i]);
        bool processed =
            ProcessOneByte(firstByte, codePoint, utf8Length) || ProcessTwoBytes(input, i, codePoint, utf8Length) ||
            ProcessThreeBytes(input, i, codePoint, utf8Length) || ProcessFourBytes(input, i, codePoint, utf8Length);
        if (processed) {
            Utf16ToUTF8Bytes(bytes, codePoint);
            i += utf8Length;
        } else {
            i++;
        }
    }
    return bytes;
}

PandaVector<uint8_t> ConvertBase64ToBytes(const PandaString &input, std::string_view encoding, size_t size)
{
    PandaString decoded;
    if (encoding == "base64url") {
        PandaString temp = input;
        std::replace(temp.begin(), temp.end(), '-', '+');
        std::replace(temp.begin(), temp.end(), '_', '/');
        decoded = base64::Decode(temp);
    } else {
        decoded = base64::Decode(input);
    }
    auto res = BytesFromString(decoded);
    res.resize(size);
    return res;
}

bool IsSymbolHex(char c)
{
    return (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || ((c >= '0') && (c <= '9'));
}

PandaString FoundInputHex(const PandaString &input, bool &error)
{
    if (input.size() == 1) {
        error = true;
        return "";
    }

    PandaString inputHex;
    bool isFirstPair = true;
    size_t idx = 1;
    while (idx < input.size()) {
        if (isFirstPair) {
            if (IsSymbolHex(input[idx - 1]) && IsSymbolHex(input[idx])) {
                inputHex += input[idx - 1];
                inputHex += input[idx];
            } else {
                error = true;
                return "";
            }
            isFirstPair = false;
        } else {
            if (IsSymbolHex(input[idx - 1]) && IsSymbolHex(input[idx])) {
                inputHex += input[idx - 1];
                inputHex += input[idx];
            } else {
                break;
            }
        }
        idx += K_HEX_PAIR_SIZE;
    }
    error = false;
    return inputHex;
}

Result<PandaVector<uint8_t>> ConvertHexToBytes(const PandaString &input)
{
    PandaVector<uint8_t> bytes;
    unsigned int arrSize = input.size();
    // 2 : means a half length of hex str's size
    for (size_t i = 0; i < arrSize / 2; i++) {
        // 2 : offset is i * 2, i * 2 + 1
        std::string hexStrTmp = {input[i * 2], input[i * 2 + 1]};
        // 2 : offset is i * 2, i * 2 + 1
        if (!IsSymbolHex(input[i * 2]) || !IsSymbolHex(input[i * 2 + 1])) {
            break;
        }
        // 16 : the base is 16
        auto value = stoi(hexStrTmp, nullptr, HEX_BASE);
        bytes.push_back(static_cast<uint8_t>(value));
    }
    return bytes;
}

Result<PandaVector<uint8_t>> ConvertStringToBytes(const PandaString &input, std::string_view encoding, size_t size)
{
    if (std::find(UTF8_ENCODINGS.begin(), UTF8_ENCODINGS.end(), encoding) != UTF8_ENCODINGS.end()) {
        return ConvertUtf8ToBytes(input);
    }
    if (std::find(ASCII_ENCODINGS.begin(), ASCII_ENCODINGS.end(), encoding) != ASCII_ENCODINGS.end()) {
        return ConvertAsciiToBytes(input);
    }
    if (std::find(UTF16_ENCODINGS.begin(), UTF16_ENCODINGS.end(), encoding) != UTF16_ENCODINGS.end()) {
        return ConvertUtf16ToBytes(input);
    }
    if (std::find(BASE64_ENCODINGS.begin(), BASE64_ENCODINGS.end(), encoding) != BASE64_ENCODINGS.end()) {
        return ConvertBase64ToBytes(input, encoding, size);
    }
    if (std::find(LATIN_ENCODINGS.begin(), LATIN_ENCODINGS.end(), encoding) != LATIN_ENCODINGS.end()) {
        return ConvertLatinToBytes(input);
    }
    if (encoding == "hex") {
        return ConvertHexToBytes(input);
    }
    return Err<PandaString>(PandaString("Unsupported encoding: ") + PandaString(encoding));
}

Result<int32_t> CalculateStringBytesLength(std::string_view input, std::string_view encoding)
{
    const int32_t kUtf16Multiplier = 2;
    const int32_t kHexDivisor = 2;
    if (std::find(K_SINGLE_BYTE_ENCODINGS.begin(), K_SINGLE_BYTE_ENCODINGS.end(), encoding) !=
        K_SINGLE_BYTE_ENCODINGS.end()) {
        return static_cast<int32_t>(input.size());
    }
    if (std::find(K_DOUBLE_BYTE_ENCODINGS.begin(), K_DOUBLE_BYTE_ENCODINGS.end(), encoding) !=
        K_DOUBLE_BYTE_ENCODINGS.end()) {
        return static_cast<int32_t>(input.size() * kUtf16Multiplier);
    }
    if (encoding == "base64" || encoding == "base64url") {
        size_t len = input.size();
        size_t pad = ((len != 0U) && input.back() == '=') ? 1 : 0;
        size_t offsetTwo = 2;
        if ((pad != 0U) && len > 1 && input[len - offsetTwo] == '=') {
            ++pad;
        }
        size_t threeLength = len * 3;
        size_t s = threeLength / base64::K_BASE64_BLOCK_SIZE;
        if (s < pad) {
            return Err<PandaString>(PandaString("Invalid base64 string: ") + PandaString(input));
        }
        size_t size = s - pad;
        return static_cast<int32_t>(size);
    }
    if (encoding == "hex") {
        if (input.size() % kHexDivisor != 0) {
            return Err<PandaString>(PandaString("Hex string must have an even length"));
        }
        return static_cast<int32_t>(input.size() / kHexDivisor);
    }
    return Err<PandaString>(PandaString("Unsupported encoding: ") + PandaString(encoding));
}

}  // namespace encoding

}  // namespace ark::ets::intrinsics::helpers
