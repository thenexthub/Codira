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
#include "plugins/ets/stdlib/native/core/IntlLocaleHelper.h"
#include "plugins/ets/stdlib/native/core/IntlLanguageTag.h"
#include "stdlib_ani_helpers.h"

#include <string>
#include <array>

namespace ark::ets::stdlib::intl {

void StdCoreVerifyBCP47LanguageTag(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale)
{
    auto localeCStr = ConvertFromAniString(env, locale);
    if (localeCStr.length() == 0) {
        std::string message = "Incorrect locale information provided";
        ThrowNewError(env, "std.core.RangeError", message.c_str(), "C{std.core.String}:");
    }
    if (!intl::IsStructurallyValidLanguageTag(localeCStr)) {
        std::string message = "Incorrect locale information provided";
        ThrowNewError(env, "std.core.RangeError", message.c_str(), "C{std.core.String}:");
    }
}

ani_status RegisterIntlLocaleHelper(ani_env *env)
{
    const auto methods = std::array {ani_native_function {
        "verifyBCP47LanguageTag", "C{std.core.String}:", reinterpret_cast<void *>(StdCoreVerifyBCP47LanguageTag)}};

    ani_class localeHelperClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.LocaleHelper", &localeHelperClass));

    return env->Class_BindStaticNativeMethods(localeHelperClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl