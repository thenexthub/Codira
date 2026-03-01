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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_ETS_PROXY_H_
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_ETS_PROXY_H_

#include <string_view>
#include <node_api.h>

#include "libarkbase/macros.h"

namespace ark::ets::interop::js::ets_proxy {

PANDA_PUBLIC_API napi_value GetETSFunction(napi_env env, std::string_view packageName, std::string_view methodName);
PANDA_PUBLIC_API napi_value GetETSClass(napi_env env, std::string_view classDescriptor);
PANDA_PUBLIC_API napi_value GetETSInstance(napi_env env, std::string_view classDescriptor);
PANDA_PUBLIC_API napi_value GetETSModule(napi_env env, const std::string &moduleName);

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_PROXY_ETS_PROXY_H_
