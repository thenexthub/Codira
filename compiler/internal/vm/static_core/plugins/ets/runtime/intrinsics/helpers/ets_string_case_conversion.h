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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_STRING_CASE_CONVERSION
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_STRING_CASE_CONVERSION

namespace ark::ets {
class EtsString;
}  // namespace ark::ets

namespace ark::ets::intrinsics::caseconversion {

EtsString *StringToUpperCase(EtsString *thisStr);
EtsString *StringToLowerCase(EtsString *thisStr);
EtsString *StringToLocaleUpperCase(EtsString *thisStr, EtsString *langTag);
EtsString *StringToLocaleLowerCase(EtsString *thisStr, EtsString *langTag);
bool DefaultLocaleAllowsFastLatinCaseConversion();

}  // namespace ark::ets::intrinsics::caseconversion

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_STRING_CASE_CONVERSION
