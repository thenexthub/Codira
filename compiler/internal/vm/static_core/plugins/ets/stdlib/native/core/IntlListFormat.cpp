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

#include "IntlListFormat.h"
#include "IntlCommon.h"
#include "libarkbase/macros.h"
#include "stdlib_ani_helpers.h"

#include "unicode/locid.h"
#include "unicode/listformatter.h"

#include <array>
#include <cassert>
#include <sstream>
#include <string>
#include <tuple>

namespace ark::ets::stdlib::intl {

namespace {
ani_string g_elementAniStr = nullptr;
ani_string g_literalAniStr = nullptr;
}  // namespace

template <typename... Args>
static void ThrowRangeError(ani_env *env, Args &&...args)
{
    std::stringstream message;
    (message << ... << args);
    ThrowNewError(env, "std.core.RangeError", message.str().c_str(), "C{std.core.String}:");
}

static icu::Locale GetLocale(const std::string &tag)
{
    auto status = UErrorCode::U_ZERO_ERROR;
    auto locale = icu::Locale::forLanguageTag(tag, status);
    if (UNLIKELY(U_FAILURE(status))) {
        return icu::Locale::getDefault();
    }
    return locale;
}

static UListFormatterWidth ToIcuStyle(ani_env *env, ani_string aniStyle)
{
    auto style = ConvertFromAniString(env, aniStyle);
    if (style == "short") {
        return UListFormatterWidth::ULISTFMT_WIDTH_SHORT;
    }
    if (style == "narrow") {
        return UListFormatterWidth::ULISTFMT_WIDTH_NARROW;
    }
    return UListFormatterWidth::ULISTFMT_WIDTH_WIDE;
}

static UListFormatterType ToIcuType(ani_env *env, ani_string aniType)
{
    auto type = ConvertFromAniString(env, aniType);
    if (type == "disjunction") {
        return UListFormatterType::ULISTFMT_TYPE_OR;
    }
    if (type == "unit") {
        return UListFormatterType::ULISTFMT_TYPE_UNITS;
    }
    return UListFormatterType::ULISTFMT_TYPE_AND;
}

static std::vector<icu::UnicodeString> ToIcuList(ani_env *env, ani_array aniList)
{
    ani_size len;
    ANI_FATAL_IF_ERROR(env->Array_GetLength(aniList, &len));

    std::vector<icu::UnicodeString> result;
    for (ani_size i = 0; i < len; i++) {
        ani_ref aniRef;
        ANI_FATAL_IF_ERROR(env->Array_Get(aniList, i, &aniRef));

        auto item = AniToUnicodeStr(env, reinterpret_cast<ani_string>(aniRef));
        result.push_back(item);
    }
    return result;
}

static ani_array ToAniArray(ani_env *env, std::vector<ani_string> strings)
{
    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ani_class stringClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.String", &stringClass));
    ani_array array;
    ANI_FATAL_IF_ERROR(env->Array_New(strings.size(), undefined, &array));
    for (size_t i = 0; i < strings.size(); ++i) {
        auto item = strings[i];
        ANI_FATAL_IF_ERROR(env->Array_Set(array, i, item));
    }
    return array;
}

ani_object FormatToParts(ani_env *env, [[maybe_unused]] ani_class klass, ani_array aniList, ani_string aniLocale,
                         ani_string aniStyle, ani_string aniType)
{
    auto list = ToIcuList(env, aniList);
    auto tag = ConvertFromAniString(env, aniLocale);
    auto locale = GetLocale(tag);
    auto style = ToIcuStyle(env, aniStyle);
    auto type = ToIcuType(env, aniType);

    auto status = UErrorCode::U_ZERO_ERROR;
    auto formatter =
        std::unique_ptr<icu::ListFormatter>(icu::ListFormatter::createInstance(locale, type, style, status));
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to create ListFormatter: ", u_errorName(status));
        return nullptr;
    }

    auto formatted = formatter->formatStringsToValue(list.data(), list.size(), status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to format list: ", u_errorName(status));
        return nullptr;
    }

    auto formattedStr = formatted.toTempString(status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to get formatted string: ", u_errorName(status));
        return nullptr;
    }

    icu::ConstrainedFieldPosition pos;
    pos.constrainCategory(UFIELD_CATEGORY_LIST);

    std::vector<ani_string> squashed;
    while (formatted.nextPosition(pos, status) != 0) {
        if (UNLIKELY(U_FAILURE(status))) {
            ThrowRangeError(env, "Error while iterating positions: ", u_errorName(status));
            return nullptr;
        }
        auto start = pos.getStart();
        auto limit = pos.getLimit();

        auto field = pos.getField();
        if (field == UListFormatterField::ULISTFMT_ELEMENT_FIELD) {
            squashed.push_back(g_elementAniStr);
        } else if (field == UListFormatterField::ULISTFMT_LITERAL_FIELD) {
            squashed.push_back(g_literalAniStr);
        } else {
            continue;
        }

        auto segment = formattedStr.tempSubStringBetween(start, limit);
        auto value = UnicodeToAniStr(env, segment);
        squashed.push_back(value);
    }
    return ToAniArray(env, squashed);
}

ani_status RegisterIntlListFormat(ani_env *env)
{
    std::array methods = {ani_native_function {
        "formatToPartsNative",
        "C{std.core.Array}C{std.core.String}C{std.core.String}C{std.core.String}:C{std.core.Object}",
        reinterpret_cast<void *>(FormatToParts)}};

    g_elementAniStr = StdStrToAni(env, "element");
    g_literalAniStr = StdStrToAni(env, "literal");

    ani_class listFormatClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.ListFormat", &listFormatClass));

    return env->Class_BindStaticNativeMethods(listFormatClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
