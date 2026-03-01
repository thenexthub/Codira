/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_PLUGINS_H_
#define PANDA_PLUGINS_PLUGINS_H_

#include "runtime/include/language_context.h"

namespace ark::plugins {

LanguageContextBase *GetLanguageContextBase(ark::panda_file::SourceLang lang);
PANDA_PUBLIC_API panda_file::SourceLang RuntimeTypeToLang(const std::string &runtimeType);
std::string_view LangToRuntimeType(panda_file::SourceLang lang);
bool HasRuntime(const std::string &runtimeType);

}  // namespace ark::plugins

#endif  // PANDA_PLUGINS_PLUGINS_H_
