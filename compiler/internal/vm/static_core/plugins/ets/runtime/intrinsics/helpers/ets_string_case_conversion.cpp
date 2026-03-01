/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ets_string_case_conversion.h"

#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/string_flatten.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_string.h"

#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"

namespace ark::ets::intrinsics::caseconversion {

namespace {

std::string_view LanguageTagToStringView(EtsString *langTag, EtsCoroutine *coroutine, PandaVector<uint8_t> *buf)
{
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> langTagHandle(coroutine, reinterpret_cast<ObjectHeader *>(langTag));
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto flatStringInfo = coretypes::FlatStringInfo::FlattenAllString(langTagHandle, ctx);

    return EtsString::FromCoreType(flatStringInfo.GetString())->ConvertToStringView(buf);
}

// The Lithuanian locale (lt) does not require any adjustment for the case conversion of characters from the Basic Latin
// block. Greek (el) does not require any adjustment for all the Latin symbols.
constexpr std::string_view localeDisallowsFastLatinConversionString = "az|tr";

bool LocaleAllowsFastLatinConversion(const icu::Locale &locale)
{
    return localeDisallowsFastLatinConversionString.find(locale.getLanguage()) == std::string_view::npos;
}

template <typename CharCnvt>
EtsString *FastConvertUtf8StringCase(const uint8_t *data, uint32_t length, CharCnvt cnvt)
{
    EtsString *res = EtsString::AllocateNonInitializedString(length, true);
    std::transform(data, data + length, res->GetDataMUtf8(), cnvt);
    return res;
}

constexpr uint8_t charCaseChangeBit = 1U << 5U;

EtsString *ToLowerCaseFast(const uint8_t *data, uint32_t length)
{
    return FastConvertUtf8StringCase(data, length, [](uint8_t ch) {
        if (ch >= 0x41 && ch <= 0x5a) {
            return static_cast<uint8_t>(ch ^ charCaseChangeBit);
        }
        return ch;
    });
}

EtsString *ToUpperCaseFast(const uint8_t *data, uint32_t length)
{
    return FastConvertUtf8StringCase(data, length, [](uint8_t ch) {
        if (ch >= 0x61 && ch <= 0x7a) {
            return static_cast<uint8_t>(ch ^ charCaseChangeBit);
        }
        return ch;
    });
}

template <typename IcuCnvt>
EtsString *IcuConvertStringCase(const coretypes::FlatStringInfo &stringInfo, const icu::Locale &locale, IcuCnvt cnvt)
{
    icu::UnicodeString utf16Str;
    if (stringInfo.IsUtf16()) {
        utf16Str = icu::UnicodeString {stringInfo.GetDataUtf16(), static_cast<int32_t>(stringInfo.GetLength())};
    } else {
        utf16Str = icu::UnicodeString {utf::Mutf8AsCString(stringInfo.GetDataUtf8()),
                                       static_cast<int32_t>(stringInfo.GetLength())};
    }
    auto res = cnvt(utf16Str, locale);
    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(res.getTerminatedBuffer()), res.length());
}

icu::UnicodeString IcuToLowerCase(icu::UnicodeString &str, const icu::Locale &locale)
{
    return str.toLower(locale);
}

icu::UnicodeString IcuToUpperCase(icu::UnicodeString &str, const icu::Locale &locale)
{
    return str.toUpper(locale);
}

UErrorCode ParseSingleBCP47LanguageTag(EtsString *langTag, EtsCoroutine *coroutine, icu::Locale &locale)
{
    if (langTag == nullptr) {
        locale = icu::Locale::getDefault();
        return U_ZERO_ERROR;
    }
    PandaVector<uint8_t> buf;
    std::string_view locTag = LanguageTagToStringView(langTag, coroutine, &buf);
    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    locale = icu::Locale::forLanguageTag(sp, status);
    return status;
}

template <typename FastCnvt, typename IcuCnvt>
EtsString *ConvertStringCase(VMHandle<coretypes::String> &strHandle, const icu::Locale &locale, FastCnvt fastCnvt,
                             IcuCnvt icuCnvt)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto flatStringInfo = coretypes::FlatStringInfo::FlattenAllString(strHandle, ctx);
    if (flatStringInfo.IsUtf8() && LocaleAllowsFastLatinConversion(locale)) {
        return fastCnvt(flatStringInfo.GetDataUtf8(), flatStringInfo.GetLength());
    }

    return IcuConvertStringCase(flatStringInfo, locale, icuCnvt);
}

template <typename FastCnvt, typename IcuCnvt>
EtsString *ConvertStringCase(EtsString *thisStr, const icu::Locale &locale, FastCnvt fastCnvt, IcuCnvt icuCnvt)
{
    if (thisStr->IsEmpty()) {
        return EtsString::CreateNewEmptyString();
    }
    // We can convert a LineString directly
    auto *string = thisStr->GetCoreType();
    if (string->IsLineString() && string->IsUtf8() && LocaleAllowsFastLatinConversion(locale)) {
        return fastCnvt(string->GetDataUtf8(), string->GetLength());
    }

    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> strHandle(coroutine, string);
    return ConvertStringCase(strHandle, locale, fastCnvt, icuCnvt);
}

template <typename FastCnvt, typename IcuCnvt>
EtsString *ConvertStringCase(EtsString *thisStr, EtsString *langTag, FastCnvt fastCnvt, IcuCnvt icuCnvt)
{
    if (thisStr->IsEmpty()) {
        return EtsString::CreateNewEmptyString();
    }

    ASSERT(langTag != nullptr);
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    icu::Locale locale;
    VMHandle<coretypes::String> langTagHandle(coroutine, langTag->GetCoreType());
    VMHandle<coretypes::String> thisStrHandle(coroutine, thisStr->GetCoreType());
    auto localeParseStatus = ParseSingleBCP47LanguageTag(langTag, coroutine, locale);
    if (UNLIKELY(U_FAILURE(localeParseStatus))) {
        auto message = "Language tag '" + ConvertToString(langTagHandle.GetPtr()) + "' is invalid or not supported";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return ConvertStringCase(thisStrHandle, locale, fastCnvt, icuCnvt);
}

}  // namespace

EtsString *StringToLowerCase(EtsString *thisStr)
{
    return ConvertStringCase(thisStr, icu::Locale::getDefault(), ToLowerCaseFast, IcuToLowerCase);
}

EtsString *StringToUpperCase(EtsString *thisStr)
{
    return ConvertStringCase(thisStr, icu::Locale::getDefault(), ToUpperCaseFast, IcuToUpperCase);
}

EtsString *StringToLocaleLowerCase(EtsString *thisStr, EtsString *langTag)
{
    return ConvertStringCase(thisStr, langTag, ToLowerCaseFast, IcuToLowerCase);
}

EtsString *StringToLocaleUpperCase(EtsString *thisStr, EtsString *langTag)
{
    return ConvertStringCase(thisStr, langTag, ToUpperCaseFast, IcuToUpperCase);
}

bool DefaultLocaleAllowsFastLatinCaseConversion()
{
    return LocaleAllowsFastLatinConversion(icu::Locale::getDefault());
}

}  // namespace ark::ets::intrinsics::caseconversion
