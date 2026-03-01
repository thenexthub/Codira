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

#include "bridge/arch/aarch64/proxy_entrypoint_aarch64.S"


PROXY_ENTRYPOINT MAKE_ASM_NAME(JSRuntimeCallJSBridge), MAKE_ASM_NAME(JSRuntimeCallJS)
PROXY_ENTRYPOINT MAKE_ASM_NAME(JSRuntimeCallJSByValueBridge), MAKE_ASM_NAME(JSRuntimeCallJSByValue)
PROXY_ENTRYPOINT MAKE_ASM_NAME(JSRuntimeNewCallJSBridge), MAKE_ASM_NAME(JSRuntimeNewCallJS)

PROXY_ENTRYPOINT MAKE_ASM_NAME(JSRuntimeCallJSQNameBridge), MAKE_ASM_NAME(JSRuntimeCallJSQName)
PROXY_ENTRYPOINT MAKE_ASM_NAME(JSRuntimeNewCallJSQNameBridge), MAKE_ASM_NAME(JSRuntimeNewCallJSQName)

PROXY_ENTRYPOINT MAKE_ASM_NAME(CallJSProxyBridge), MAKE_ASM_NAME(CallJSProxy)
PROXY_ENTRYPOINT MAKE_ASM_NAME(CallJSFunctionBridge), MAKE_ASM_NAME(CallJSFunction)