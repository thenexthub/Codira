/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "include/object_header.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/intrinsics/helpers/array_buffer_helper.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"

using namespace std::string_view_literals;
constexpr std::array UTF8_ENCODINGS = {"utf8"sv, "utf-8"sv, "ascii"sv};
constexpr std::array UTF16_ENCODINGS = {"utf16le"sv, "ucs2"sv, "ucs-2"sv};
constexpr std::array BASE64_ENCODINGS = {"base64"sv, "base64url"sv};
constexpr std::array LATIN_ENCODINGS = {"latin1"sv, "binary"sv};
constexpr size_t TWO_BYTES = 2;
constexpr size_t THREE_BYTES = 3;

namespace ark::ets::intrinsics {

/// @brief Creates a new ArrayBuffer with specified size and optional initial data
static EtsHandle<EtsEscompatArrayBuffer> CreateArrayBuffer(EtsCoroutine *coro, EtsInt byteLength,
                                                           const uint8_t *data = nullptr)
{
    EtsHandle<EtsEscompatArrayBuffer> newBuffer(coro, EtsEscompatArrayBuffer::Create(coro, byteLength));
    if (UNLIKELY(newBuffer.GetPtr() == nullptr)) {
        return newBuffer;
    }
    auto *buffer = newBuffer->GetData<uint8_t *>();
    if (LIKELY(data != nullptr && buffer != nullptr && byteLength > 0)) {
        std::copy_n(data, byteLength, buffer);
    }
    return newBuffer;
}

static size_t ByteLength(const PandaString &string, const PandaString &encoding)
{
    size_t length = string.length();
    if (length == 0) {
        return 0;
    }
    size_t siz = length;
    if (std::find(BASE64_ENCODINGS.begin(), BASE64_ENCODINGS.end(), encoding) != BASE64_ENCODINGS.end()) {
        size_t pos = 0;
        size_t byteLength = length;
        while (byteLength > 1 && (pos = string.find('=', pos)) != std::string::npos) {
            byteLength--;
            pos++;
        }
        siz = (byteLength * THREE_BYTES) >> TWO_BYTES;
    }
    return siz;
}

/**
 * @brief Converts an ETS string to bytes using specified encoding
 * @details Supported encodings:
 *   - UTF-8/UTF-8
 *   - ASCII (7-bit)
 *   - UTF-16LE/UCS2
 *   - Base64/Base64URL
 *   - Latin1/Binary
 *   - Hex (must be even length)
 */
static PandaVector<uint8_t> ConvertEtsStringToBytes(EtsString *strObj, EtsString *encodingObj, EtsCoroutine *coro,
                                                    const LanguageContext &ctx)
{
    PandaVector<uint8_t> strBuf;
    PandaVector<uint8_t> encodingBuf;
    PandaString input(strObj->ConvertToStringView(&strBuf));
    PandaString encoding =
        encodingObj != nullptr ? PandaString(encodingObj->ConvertToStringView(&encodingBuf)) : "utf8";
    size_t siz = ByteLength(input, encoding);
    auto res = helpers::encoding::ConvertStringToBytes(input, encoding, siz);
    if (std::holds_alternative<helpers::Err<PandaString>>(res)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(res).Message().c_str()));
        return PandaVector<uint8_t> {};
    }
    return std::get<PandaVector<uint8_t>>(res);
}

/// @brief Calculates byte length of string when encoded
extern "C" EtsInt EtsStringBytesLength(EtsString *strObj, EtsString *encodingObj)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    PandaVector<uint8_t> strBuf;
    PandaVector<uint8_t> encodingBuf;
    PandaString input(strObj->ConvertToStringView(&strBuf));
    PandaString encoding =
        encodingObj != nullptr ? PandaString(encodingObj->ConvertToStringView(&encodingBuf)) : "utf8";
    auto res = helpers::encoding::CalculateStringBytesLength(input, encoding);
    if (std::holds_alternative<helpers::Err<PandaString>>(res)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(res).Message().c_str()));
        return -1;
    }
    return std::get<int32_t>(res);
}

/// @brief Creates ArrayBuffer from encoded string
extern "C" EtsEscompatArrayBuffer *EtsArrayBufferFromEncodedString(EtsString *strObj, EtsString *encodingObj)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    PandaVector<uint8_t> bytes = ConvertEtsStringToBytes(strObj, encodingObj, coro, ctx);
    auto byteLength = static_cast<EtsInt>(bytes.size());
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsEscompatArrayBuffer> newBuffer =
        CreateArrayBuffer(coro, byteLength, byteLength > 0 ? bytes.data() : nullptr);
    if (newBuffer.GetPtr() == nullptr) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    return newBuffer.GetPtr();
}

extern "C" void EtsEscompatArrayBufferSetValues(EtsEscompatArrayBuffer *arrayBuffer, EtsEscompatArrayBuffer *other,
                                                EtsInt begin)
{
    arrayBuffer->SetValues(other, begin);
}

extern "C" EtsByte EtsEscompatArrayBufferAt(EtsEscompatArrayBuffer *arrayBuffer, EtsInt pos)
{
    return arrayBuffer->At(pos);
}

extern "C" void EtsEscompatArrayBufferSet(EtsEscompatArrayBuffer *arrayBuffer, EtsInt pos, EtsByte val)
{
    arrayBuffer->Set(pos, val);
}

extern "C" ObjectHeader *EtsEscompatArrayBufferAllocateNonMovable(EtsInt length)
{
    return EtsEscompatArrayBuffer::AllocateNonMovableArray(length)->GetCoreType();
}

extern "C" EtsLong EtsEscompatArrayBufferGetAddress(ObjectHeader *byteArray)
{
    ASSERT(byteArray != nullptr);
    return EtsEscompatArrayBuffer::GetAddress(reinterpret_cast<EtsByteArray *>(byteArray));
}

/// @brief Creates new ArrayBuffer from slice of existing buffer
extern "C" EtsEscompatArrayBuffer *EtsArrayBufferFromBufferSlice(EtsEscompatArrayBuffer *obj, EtsInt offset,
                                                                 EtsInt length)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsEscompatArrayBuffer> original(coro, obj);
    if (original->WasDetached()) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8("ArrayBuffer was detached"));
        return nullptr;
    }
    ASSERT(original->GetData() != nullptr);

    EtsInt origByteLength = original->GetByteLength();
    if (offset < 0 || offset > origByteLength) {
        ThrowException(ctx, coro, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Offset is out of bounds"));
        return nullptr;
    }
    if (length < 0 || length > origByteLength - offset) {
        ThrowException(ctx, coro, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Length is out of bounds"));
        return nullptr;
    }

    EtsHandle<EtsEscompatArrayBuffer> newBuffer = CreateArrayBuffer(coro, length);
    if (newBuffer.GetPtr() == nullptr) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    std::copy_n(std::next(reinterpret_cast<uint8_t *>(original->GetData()), offset), length,
                reinterpret_cast<uint8_t *>(newBuffer->GetData()));
    return newBuffer.GetPtr();
}

/**
 * @brief Converts ArrayBuffer content to string using specified encoding.
 *
 * This function validates the input buffer and indices, extracts the requested bytes,
 * determines the encoding, and then converts the byte sequence into the desired string.
 */
extern "C" EtsString *EtsArrayBufferToString(EtsEscompatArrayBuffer *buffer, EtsString *encodingObj, EtsInt start,
                                             EtsInt end)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);

    auto vb = helpers::encoding::ValidateBuffer(buffer);
    if (std::holds_alternative<helpers::Err<PandaString>>(vb)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(vb).Message().c_str()));
        return nullptr;
    }

    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsEscompatArrayBuffer> buf(coro, buffer);
    EtsInt byteLength = buf->GetByteLength();

    auto vi = helpers::encoding::ValidateIndices(byteLength, start, end);
    if (std::holds_alternative<helpers::Err<PandaString>>(vi)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(vi).Message().c_str()));
        return nullptr;
    }

    auto length = static_cast<size_t>(end - start);
    PandaVector<uint8_t> bytes(length);
    if (buf->GetData() != nullptr) {
        std::copy_n(std::next(reinterpret_cast<uint8_t *>(buf->GetData()), start), length, bytes.data());
    }

    PandaVector<uint8_t> encodingBuf;
    PandaString encoding =
        encodingObj != nullptr ? PandaString(encodingObj->ConvertToStringView(&encodingBuf)) : "utf8";

    PandaString output;
    if (std::find(UTF8_ENCODINGS.begin(), UTF8_ENCODINGS.end(), encoding) != UTF8_ENCODINGS.end()) {
        output = helpers::encoding::ConvertUtf8Encoding(bytes);
    } else if (std::find(UTF16_ENCODINGS.begin(), UTF16_ENCODINGS.end(), encoding) != UTF16_ENCODINGS.end()) {
        output = helpers::encoding::ConvertUtf16Encoding(bytes);
    } else if (std::find(BASE64_ENCODINGS.begin(), BASE64_ENCODINGS.end(), encoding) != BASE64_ENCODINGS.end()) {
        output = helpers::encoding::ConvertBase64Encoding(bytes, encoding);
    } else if (encoding == "hex") {
        output = helpers::encoding::ConvertHexEncoding(bytes);
    } else if (std::find(LATIN_ENCODINGS.begin(), LATIN_ENCODINGS.end(), encoding) != LATIN_ENCODINGS.end()) {
        output = helpers::encoding::ConvertLatinEncoding(bytes);
    } else {
        PandaString errMsg = "Unsupported encoding: " + encoding;
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(errMsg.c_str()));
        return nullptr;
    }
    return EtsString::CreateFromUtf8(output.data(), output.size());
}

extern "C" EtsObject *EtsArrayBufferRegisterWeakRef(EtsEscompatArrayBuffer *buffer, EtsLong finalizer,
                                                    EtsLong finalizerData)
{
    using FinalizerCallback = void (*)(void *);

    auto *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsObject> bufferHandle(coro, buffer);
    return coro->GetPandaVM()->RegisterFinalizerForObject(
        coro, bufferHandle, reinterpret_cast<FinalizerCallback>(finalizer), reinterpret_cast<void *>(finalizerData));
}

extern "C" EtsLong EtsArrayBufferUnregisterWeakRef(EtsObject *weakRef)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *finalizableWeakRef = EtsFinalizableWeakRef::FromEtsObject(weakRef);
    if (LIKELY(coro->GetPandaVM()->UnregisterFinalizerForObject(coro, finalizableWeakRef))) {
        auto finalizer = finalizableWeakRef->ReleaseFinalizer();
        return reinterpret_cast<EtsLong>(finalizer.GetFinalizerArg());
    }
    return 0U;
}

}  // namespace ark::ets::intrinsics
