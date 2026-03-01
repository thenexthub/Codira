/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_string.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"

namespace ark::ets {

/* static */
EtsString *EtsString::CreateFromMUtf8(const char *mutf8)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (mutf8 == nullptr) {
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return nullptr;
    }
    const auto *data = reinterpret_cast<const uint8_t *>(mutf8);
    coretypes::LineString *s = coretypes::LineString::CreateFromMUtf8(data, ctx, Runtime::GetCurrent()->GetPandaVM());
    return reinterpret_cast<EtsString *>(s);
}

/* static */
EtsString *EtsString::CreateFromMUtf8(const char *mutf8, uint32_t utf16Length)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (utf16Length == 0) {
        return reinterpret_cast<EtsString *>(
            coretypes::LineString::CreateEmptyLineString(ctx, Runtime::GetCurrent()->GetPandaVM()));
    }
    if (mutf8 == nullptr) {
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return nullptr;
    }
    const auto *data = reinterpret_cast<const uint8_t *>(mutf8);
    coretypes::LineString *s =
        coretypes::LineString::CreateFromMUtf8(data, utf16Length, ctx, Runtime::GetCurrent()->GetPandaVM());
    return reinterpret_cast<EtsString *>(s);
}

/* static */
EtsString *EtsString::CreateFromUtf8(const char *utf8, uint32_t length)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (length == 0) {
        return reinterpret_cast<EtsString *>(
            coretypes::LineString::CreateEmptyLineString(ctx, Runtime::GetCurrent()->GetPandaVM()));
    }
    if (utf8 == nullptr) {
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return nullptr;
    }
    const auto *data = reinterpret_cast<const uint8_t *>(utf8);
    return reinterpret_cast<EtsString *>(
        coretypes::LineString::CreateFromUtf8(data, length, ctx, Runtime::GetCurrent()->GetPandaVM()));
}

/* static */
EtsString *EtsString::CreateFromAscii(const char *str, uint32_t length)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    const auto *data = reinterpret_cast<const uint8_t *>(str);
    return reinterpret_cast<EtsString *>(
        coretypes::LineString::CreateFromMUtf8(data, length, length, true, ctx, Runtime::GetCurrent()->GetPandaVM()));
}

/* static */
EtsString *EtsString::CreateFromUtf16(const EtsChar *utf16, uint32_t length)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (length == 0) {
        return reinterpret_cast<EtsString *>(
            coretypes::LineString::CreateEmptyLineString(ctx, Runtime::GetCurrent()->GetPandaVM()));
    }
    if (utf16 == nullptr) {
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return nullptr;
    }
    coretypes::LineString *s =
        coretypes::LineString::CreateFromUtf16(utf16, length, ctx, Runtime::GetCurrent()->GetPandaVM());
    return reinterpret_cast<EtsString *>(s);
}

/* static */
EtsString *EtsString::CreateNewStringFromCharCode(EtsDouble charCode)
{
    constexpr size_t SINGLE_CHAR_LENGTH = 1U;
    uint16_t character = CodeToChar(charCode);
    bool compress = Runtime::GetOptions().IsRuntimeCompressedStringsEnabled() && IsASCIICharacter(character);
    EtsString *s = AllocateNonInitializedString(SINGLE_CHAR_LENGTH, compress);
    auto putCharacterIntoString = [character](auto *dstData) {
        Span<std::remove_pointer_t<decltype(dstData)>> to(dstData, SINGLE_CHAR_LENGTH);
        to[0] = character;
    };
    if (compress) {
        putCharacterIntoString(s->GetDataMUtf8());
    } else {
        putCharacterIntoString(s->GetDataUtf16());
    }
    return s;
}

/* static */
EtsString *EtsString::CreateNewStringFromChars(uint32_t offset, uint32_t length, EtsArray *chararray)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    return reinterpret_cast<EtsString *>(coretypes::LineString::CreateNewLineStringFromChars(
        offset, length, reinterpret_cast<coretypes::Array *>(chararray), ctx, Runtime::GetCurrent()->GetPandaVM()));
}

/* static */
EtsString *EtsString::CreateNewStringFromBytes(EtsArray *bytearray, EtsInt high, EtsInt offset, EtsInt length)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (length == 0) {
        return reinterpret_cast<EtsString *>(
            coretypes::LineString::CreateEmptyLineString(ctx, Runtime::GetCurrent()->GetPandaVM()));
    }
    if (bytearray == nullptr) {
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return nullptr;
    }
    if (offset < 0 || length < 0 || bytearray->GetLength() < static_cast<uint32_t>(offset + length)) {
        ThrowArrayIndexOutOfBoundsException(static_cast<uint32_t>(offset + length), bytearray->GetLength(), ctx,
                                            ManagedThread::GetCurrent());
        return nullptr;
    }
    return reinterpret_cast<EtsString *>(coretypes::LineString::CreateNewLineStringFromBytes(
        offset, length, static_cast<uint32_t>(high), reinterpret_cast<coretypes::Array *>(bytearray), ctx,
        Runtime::GetCurrent()->GetPandaVM()));
}

/* static */
EtsString *EtsString::CreateNewStringFromString(EtsString *etsString)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (etsString == nullptr) {
        ThrowNullPointerException(ctx, ManagedThread::GetCurrent());
        return nullptr;
    }
    coretypes::String *string = etsString->GetCoreType();
    return reinterpret_cast<EtsString *>(
        coretypes::String::CreateFromString(string, ctx, Runtime::GetCurrent()->GetPandaVM()));
}

/* static */
EtsString *EtsString::CreateNewEmptyString()
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    return reinterpret_cast<EtsString *>(
        coretypes::LineString::CreateEmptyLineString(ctx, Runtime::GetCurrent()->GetPandaVM()));
}

EtsBoolean EtsString::EndsWith(EtsString *suffix, EtsInt endIndex)
{
    ASSERT(suffix != nullptr);
    if (suffix->IsEmpty()) {
        return ToEtsBoolean(true);
    }
    auto strLen = GetLength();
    if (strLen == 0) {
        return ToEtsBoolean(false);
    }
    if (endIndex <= 0) {
        return ToEtsBoolean(false);
    }
    if (endIndex > strLen) {
        endIndex = strLen;
    }
    ASSERT(endIndex > 0);
    auto suffixLen = suffix->GetLength();
    auto fromIndex = endIndex - suffixLen;
    if (fromIndex < 0) {
        return ToEtsBoolean(false);
    }
    auto *thisCoreType = GetCoreType();
    auto *suffCoreType = suffix->GetCoreType();
    for (EtsInt i = 0; i < suffixLen; ++i) {
        if (thisCoreType->At(fromIndex + i) != suffCoreType->At(i)) {
            return ToEtsBoolean(false);
        }
    }
    return ToEtsBoolean(true);
}

EtsBoolean EtsString::StartsWith(EtsString *prefix, EtsInt fromIndex)
{
    ASSERT(prefix != nullptr);
    if (fromIndex < 0) {
        fromIndex = 0;
    }
    auto prefixLen = prefix->GetLength();
    ASSERT(prefixLen >= 0);
    if (fromIndex > GetLength() - prefixLen) {
        return ToEtsBoolean(prefix->IsEmpty());
    }
    auto *thisCoreType = GetCoreType();
    auto *prefCoreType = prefix->GetCoreType();
    for (EtsInt i = 0; i < prefixLen; ++i) {
        if (thisCoreType->At<false>(fromIndex + i) != prefCoreType->At<false>(i)) {
            return ToEtsBoolean(false);
        }
    }
    return ToEtsBoolean(true);
}

EtsString *EtsString::TrimLeft()
{
    if (IsEmpty() || !utf::IsWhiteSpaceChar(At(0))) {
        return this;
    }
    const auto numChars = GetLength();
    auto notWhiteSpaceIndex = numChars;
    for (int i = 1; i < numChars; ++i) {
        if (!utf::IsWhiteSpaceChar(At(i))) {
            notWhiteSpaceIndex = i;
            break;
        }
    }
    auto len = numChars - notWhiteSpaceIndex;
    return FastSubString(this, static_cast<uint32_t>(notWhiteSpaceIndex), static_cast<uint32_t>(len));
}

EtsString *EtsString::TrimRight()
{
    if (IsEmpty()) {
        return this;
    }
    auto last = GetLength() - 1;
    if (!utf::IsWhiteSpaceChar(At(last))) {
        return this;
    }
    int notWhiteSpaceIndex = -1;
    for (int i = last - 1; i >= 0; --i) {
        if (!utf::IsWhiteSpaceChar(At(i))) {
            notWhiteSpaceIndex = i;
            break;
        }
    }
    auto len = notWhiteSpaceIndex + 1;
    return EtsString::FastSubString(this, 0, static_cast<uint32_t>(len));
}

EtsString *EtsString::Trim()
{
    if (IsEmpty()) {
        return this;
    }
    int left = 0;
    int right = GetLength() - 1;
    while (utf::IsWhiteSpaceChar(At(right))) {
        if (right == left) {
            return EtsString::CreateNewEmptyString();
        }
        --right;
    }
    while (left < right && utf::IsWhiteSpaceChar(At(left))) {
        ++left;
    }
    return EtsString::FastSubString(this, left, static_cast<uint32_t>(right - left + 1));
}

std::string_view EtsString::ConvertToStringView([[maybe_unused]] PandaVector<uint8_t> *buf)
{
    if (IsUtf8()) {
        return {utf::Mutf8AsCString(IsTreeString() ? GetTreeStringDataMUtf8(*buf) : GetDataMUtf8()),
                static_cast<size_t>(GetLength())};
    }

    PandaVector<uint16_t> tree16Buf;
    size_t len = utf::Utf16ToUtf8Size(IsTreeString() ? GetTreeStringDataUtf16(tree16Buf) : GetDataUtf16(),
                                      GetUtf16Length(), false) -
                 1;
    buf->reserve(len);
    utf::ConvertRegionUtf16ToUtf8(IsTreeString() ? tree16Buf.data() : GetDataUtf16(), buf->data(), GetUtf16Length(),
                                  len, 0, false);
    return {utf::Mutf8AsCString(buf->data()), len};
}

/* static */
EtsString *EtsString::DoReplace(EtsString *src, EtsChar oldC, EtsChar newC)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    if (src->GetLength() == 0) {
        return reinterpret_cast<EtsString *>(
            coretypes::LineString::CreateEmptyLineString(ctx, Runtime::GetCurrent()->GetPandaVM()));
    }
    coretypes::String *result = coretypes::String::DoReplace(reinterpret_cast<coretypes::String *>(src), oldC, newC,
                                                             ctx, Runtime::GetCurrent()->GetPandaVM());
    return reinterpret_cast<EtsString *>(result);
}

}  // namespace ark::ets
