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
#ifndef PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_PEEPHOLE_ETS_INL_H
#define PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_PEEPHOLE_ETS_INL_H

static bool PeepholeStringEquals(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeStringSubstring(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeLdObjByName(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeStObjByName(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeEquals(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeStrictEquals(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeTypeof(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeDoubleToString(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeGetTypeInfo(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeNullcheck(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeStringFromCharCodeSingle(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeIstrue(GraphVisitor *v, IntrinsicInst *intrinsic);

#ifdef PANDA_ETS_INTEROP_JS
bool TryFuseGetPropertyAndCast(IntrinsicInst *intrinsic, RuntimeInterface::IntrinsicId newId);
static bool PeepholeJSRuntimeGetValueString(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeJSRuntimeGetValueDouble(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeJSRuntimeGetValueBoolean(GraphVisitor *v, IntrinsicInst *intrinsic);
bool TryFuseCastAndSetProperty(IntrinsicInst *intrinsic, RuntimeInterface::IntrinsicId newId);
static bool PeepholeJSRuntimeNewJSValueString(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeJSRuntimeNewJSValueDouble(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeJSRuntimeNewJSValueBoolean(GraphVisitor *v, IntrinsicInst *intrinsic);
static bool PeepholeResolveQualifiedJSCall(GraphVisitor *v, IntrinsicInst *intrinsic);
#endif
#endif  // PANDA_PLUGINS_ETS_COMPILER_INTRINSICS_PEEPHOLE_ETS_INL_H
