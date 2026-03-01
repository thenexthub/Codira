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

#ifndef PANDA_PLUGINS_ETS_COMPILER_ETS_CODEGEN_EXTENSIONS_H
#define PANDA_PLUGINS_ETS_COMPILER_ETS_CODEGEN_EXTENSIONS_H

bool ResolveCallByNameCodegen(ResolveVirtualInst *resolver);

void EtsGetNativeMethod(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsGetNativeMethodManagedClass(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsGetMethodNativePointer(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsGetNativeApiEnv(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsBeginNativeMethod(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsEndNativeMethod(IntrinsicInst *inst, Reg dst, SRCREGS &src, bool isObject);
void EtsEndNativeMethodPrim(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsEndNativeMethodObj(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsCheckNativeException(IntrinsicInst *inst, Reg dst, SRCREGS &src);
void EtsWrapObjectNative(WrapObjectNativeInst *wrapObject);

#endif  // PANDA_PLUGINS_ETS_COMPILER_ETS_CODEGEN_EXTENSIONS_H
