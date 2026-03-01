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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLLOCALEHELPER_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLLOCALEHELPER_H

#include "stdlib_ani_helpers.h"

#include <ani.h>
#include <string>

namespace ark::ets::stdlib::intl {

ani_status RegisterIntlLocaleHelper(ani_env *env);

void StdCoreVerifyBCP47LanguageTag(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale);

}  // namespace ark::ets::stdlib::intl

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLLOCALEHELPER_H
