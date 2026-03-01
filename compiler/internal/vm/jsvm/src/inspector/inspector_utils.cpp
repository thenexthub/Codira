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

#include "inspector_utils.h"

#include "jsvm_dfx.h"
#include "unicode/unistr.h"

#if HAVE_OPENSSL
#include "openssl/opensslv.h"
#endif

#if OPENSSL_VERSION_MAJOR >= 3
#include "openssl/provider.h"
#endif

#include <openssl/err.h>
#include <openssl/rand.h>

namespace jsvm {
namespace inspector {
namespace {
inline icu::UnicodeString Utf8ToUtf16(const char* data, size_t len)
{
    icu::UnicodeString utf16Str = icu::UnicodeString::fromUTF8(icu::StringPiece(data, len));

    return utf16Str;
}

inline std::string Utf16ToUtf8(const char16_t* data, size_t length)
{
    icu::UnicodeString unicodeStr(data, length);
    std::string utf8Str;
    unicodeStr.toUTF8String(utf8Str);

    return utf8Str;
}
} // namespace

std::unique_ptr<v8_inspector::StringBuffer> Utf8ToStringView(const std::string_view message)
{
    icu::UnicodeString utf16Str = Utf8ToUtf16(message.data(), message.length());
    size_t utf16Len = utf16Str.length();

    v8_inspector::StringView view(reinterpret_cast<const uint16_t*>(utf16Str.getBuffer()), utf16Len);
    return v8_inspector::StringBuffer::create(view);
}

std::string StringViewToUtf8(v8_inspector::StringView view)
{
    if (view.length() == 0) {
        return "";
    }
    if (view.is8Bit()) {
        return std::string(reinterpret_cast<const char*>(view.characters8()), view.length());
    }
    const char16_t* source = reinterpret_cast<const char16_t*>(view.characters16());

    return Utf16ToUtf8(source, view.length());
}

static constexpr char BASE64_CHAR_SET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// clang-format off
size_t Base64Encode(const char* inputString, size_t slen, char* outputBuffer, size_t dlen)
{
    // 1: caluate encode size and check
    size_t strLen = slen;
    size_t encodedStrLen = Base64EncodeSize(slen);

    CHECK_GE(dlen, encodedStrLen);

    // 2: the index do not exceed the range of outputBuffer and form a complete four-character block
    for (size_t i = 0, j = 0; j < strLen - 2; i += TRANSFORMED_CHAR_NUM, j += TO_TRANSFORM_CHAR_NUM) {
        // convert three 8bit into four 6bit; then add two 0 bit in each 6 bit
        // former 00 + first 6 bits of the first char
        outputBuffer[i] = BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[j]) & 0xff) >> ByteOffset::BIT_2];
        // 00 + the last 2 bits of the first char + the first 4 bits of the second char
        outputBuffer[i + ByteOffset::BYTE_1] =
            BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[j]) & 0x03) << ByteOffset::BIT_4 |
                            (static_cast<unsigned int>(inputString[j + ByteOffset::BYTE_1]) & 0xf0) >>
                                ByteOffset::BIT_4];
        // 00 + last 4 bits of the second char + the first 2 bits of the third char
        outputBuffer[i + ByteOffset::BYTE_2] =
            BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[j + ByteOffset::BYTE_1]) & 0x0f) <<
                                ByteOffset::BIT_2 |
                            (static_cast<unsigned int>(inputString[j + ByteOffset::BYTE_2]) & 0xc0) >>
                                ByteOffset::BIT_6];
        // 00 + the last 6 bits of the third char
        outputBuffer[i + ByteOffset::BYTE_3] =
            BASE64_CHAR_SET[static_cast<unsigned int>(inputString[j + ByteOffset::BYTE_2]) & 0x3f];
    }
    switch (strLen % TO_TRANSFORM_CHAR_NUM) {
        // the original string is less than three bytes, and the missing place is filled with '=' to patch four bytes
        case ByteSize::SIZE_1_BYTES:
            // 1,2: the original character is one, and two characters are missing after conversion
            outputBuffer[encodedStrLen - ByteOffset::BYTE_4] =
                BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[strLen - ByteOffset::BYTE_1]) & 0xff) >>
                                ByteOffset::BIT_2];
            outputBuffer[encodedStrLen - ByteOffset::BYTE_3] =
                BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[strLen - ByteOffset::BYTE_1]) & 0x03) <<
                                ByteOffset::BIT_4];
            outputBuffer[encodedStrLen - ByteOffset::BYTE_2] = '=';
            outputBuffer[encodedStrLen - ByteOffset::BYTE_1] = '=';
            break;
        case ByteSize::SIZE_2_BYTES:
            // 1: the original character is two, and a character are missing after conversion
            outputBuffer[encodedStrLen - ByteOffset::BYTE_4] =
                BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[strLen - ByteOffset::BYTE_2]) & 0xff) >>
                                ByteOffset::BIT_2];
            outputBuffer[encodedStrLen - ByteOffset::BYTE_3] =
                BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[strLen - ByteOffset::BYTE_2]) & 0x03) <<
                                ByteOffset::BIT_4 |
                                (static_cast<unsigned int>(inputString[strLen - ByteOffset::BYTE_1]) & 0xf0) >>
                                    ByteOffset::BIT_4];
            outputBuffer[encodedStrLen - ByteOffset::BYTE_2] =
                BASE64_CHAR_SET[(static_cast<unsigned int>(inputString[strLen - ByteOffset::BYTE_1]) & 0x0f) <<
                                ByteOffset::BIT_2];
            outputBuffer[encodedStrLen - ByteOffset::BYTE_1] = '=';
            break;
        default:
            break;
    }

    return encodedStrLen;
}
// clang-format on

std::string GetHumanReadableProcessName()
{
    return "JSVM[" + std::to_string(platform::OS::GetPid()) + "]";
}

MUST_USE_RESULT bool CSPRNG(void* buffer, size_t length)
{
    unsigned char* buf = static_cast<unsigned char*>(buffer);
    do {
        if (RAND_status() == 1) {
#if OPENSSL_VERSION_MAJOR >= 3
            if (RAND_bytes_ex(nullptr, buf, length, 0) == 1) {
                return true;
            }
#else
            while (length > INT_MAX && RAND_bytes(buf, INT_MAX) == 1) {
                buf += INT_MAX;
                length -= INT_MAX;
            }
            if (length <= INT_MAX && RAND_bytes(buf, static_cast<int>(length)) == 1) {
                return true;
            }
#endif
        }
#if OPENSSL_VERSION_MAJOR >= 3
        const auto code = ERR_peek_last_error();
        // A misconfigured OpenSSL 3 installation may report 1 from RAND_poll()
        // and RAND_status() but fail in RAND_bytes() if it cannot look up
        // a matching algorithm for the CSPRNG.
        if (ERR_GET_LIB(code) == ERR_LIB_RAND) {
            const auto reason = ERR_GET_REASON(code);
            if (reason == RAND_R_ERROR_INSTANTIATING_DRBG || reason == RAND_R_UNABLE_TO_FETCH_DRBG ||
                reason == RAND_R_UNABLE_TO_CREATE_DRBG) {
                return false;
            }
        }
#endif
    } while (RAND_poll() == 1);

    return false;
}

void CheckedUvLoopClose(uv_loop_t* loop)
{
    if (uv_loop_close(loop) == 0) {
        return;
    }

    // Finally, abort.
    UNREACHABLE("uv_loop_close() while having open handles");
}

using v8::Isolate;
using v8::Local;
using v8::String;
using v8::Value;

TwoByteValue::TwoByteValue(Isolate* isolate, Local<Value> value)
{
    if (value.IsEmpty()) {
        return;
    }

    Local<String> string;
    if (!value->ToString(isolate->GetCurrentContext()).ToLocal(&string)) {
        return;
    }

    // Allocate enough space to include the null terminator
    const size_t storage = string->Length() + 1;
    AllocateSufficientStorage(storage);

    const int flags = String::NO_NULL_TERMINATION;
    const int length = string->Write(isolate, Out(), 0, storage, flags);
    SetLengthAndZeroTerminate(length);
}
} // namespace inspector
} // namespace jsvm