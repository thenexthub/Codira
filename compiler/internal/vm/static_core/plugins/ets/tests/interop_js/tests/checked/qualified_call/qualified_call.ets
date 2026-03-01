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

import { Outer, outerObj }
    from "checked/qualified_call/qualified_call.js"

//! CHECKER      Call method of class member INT
//! RUN          entry: "callInner", options: "--compiler-enable-jit=false"

//! CHECKER      Call method of class member JIT (slow)
//! RUN          force_jit: true, entry: "callInner", options: "--compiler-regex=qualified_call.ETSGLOBAL::callInner --compiler-enable-fast-interop=false"
//! METHOD       "callInner"
//! PASS_AFTER   "Codegen"

//! CHECKER      Call method of class member JIT (fast)
//! RUN          force_jit: true, entry: "callInner", options: "--compiler-regex=qualified_call.ETSGLOBAL::callInner --compiler-interop-try-single-scope=false"
//! METHOD       "callInner"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    /LoadAndInitClass.*jscall/
//! INST_NEXT    /LoadStatic.*jscall.qname_start_from/
//! INST_NEXT    "Intrinsic.CompilerInitJSCallClassForCtx"
//! INST_NEXT    "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT    "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NEXT    "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT    "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT    "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT    "Intrinsic.CompilerDestroyLocalScope"
//! PASS_AFTER   "Cleanup"
//! PASS_AFTER_NEXT "Peepholes"
//! INST_NOT      "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST          "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT     /LoadAndInitClass.*jscall/
//! INST_NEXT     /LoadStatic.*jscall.qname_start_from/
//! INST_NEXT     "Intrinsic.CompilerInitJSCallClassForCtx"
//! INST_NEXT     "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT     "Intrinsic.CompilerLoadJSConstantPool"
//! INST_NEXT     "Intrinsic.CompilerGetJSElement"
//! INST_NEXT     "Intrinsic.CompilerGetJSProperty"
//! INST_NEXT     "Intrinsic.CompilerGetJSElement"
//! INST_NEXT     "Intrinsic.CompilerGetJSProperty"
//! INST_NEXT     "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT     "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT     "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT     "Intrinsic.CompilerDestroyLocalScope"
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.CompilerLoadJSConstantPool", 1
//! # 1 `get` for new and 2 `get`s for call
//! INST_COUNT    "Intrinsic.CompilerGetJSElement", 3
//! INST_COUNT    "Intrinsic.CompilerGetJSProperty", 3

//! CHECKER      Call method of class member PAOC (slow)
//! RUN_PAOC     options: "--compiler-regex=qualified_call.ETSGLOBAL::callInner --compiler-enable-fast-interop=false"
//! METHOD       "callInner"
//! PASS_AFTER   "Codegen"
//! RUN          entry: "callInner"

//! CHECKER      Call method of class member LLVMAOT (slow)
//! RUN_LLVM     options: "--compiler-regex=qualified_call.ETSGLOBAL::callInner --compiler-enable-fast-interop=false"
//! RUN          entry: "callInner"

//! CHECKER      Call method of class member AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=qualified_call.ETSGLOBAL::callInner --compiler-interop-try-single-scope=false"
//! METHOD       "callInner"
//! PASS_AFTER   "IrBuilder"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    /LoadAndInitClass.*jscall/
//! INST_NEXT    /LoadStatic.*jscall.qname_start_from/
//! INST_NEXT    "Intrinsic.CompilerInitJSCallClassForCtx"
//! INST_NEXT    "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT    "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NEXT    "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT    "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT    "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT    "Intrinsic.CompilerDestroyLocalScope"
//! PASS_AFTER   "Cleanup"
//! PASS_AFTER_NEXT "Peepholes"
//! INST_NOT      "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST          "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT     /LoadAndInitClass.*jscall/
//! INST_NEXT     /LoadStatic.*jscall.qname_start_from/
//! INST_NEXT     "Intrinsic.CompilerInitJSCallClassForCtx"
//! INST_NEXT     "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT     "Intrinsic.CompilerLoadJSConstantPool"
//! INST_NEXT     "Intrinsic.CompilerGetJSElement"
//! INST_NEXT     "Intrinsic.CompilerGetJSProperty"
//! INST_NEXT     "Intrinsic.CompilerGetJSElement"
//! INST_NEXT     "Intrinsic.CompilerGetJSProperty"
//! INST_NEXT     "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT     "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT     "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT     "Intrinsic.CompilerDestroyLocalScope"
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.CompilerLoadJSConstantPool", 1
//! # 1 `get` for new and 2 `get`s for call
//! INST_COUNT    "Intrinsic.CompilerGetJSElement", 3
//! INST_COUNT    "Intrinsic.CompilerGetJSProperty", 3
//! RUN           entry: "callInner"

function callInner(): int {
    let outer = new Outer(321)
    if (outer.inner.getInt() as int == 321) {
        return 0;
    }
    return 1;
}

//! CHECKER      Call method of object nested member INT
//! RUN          entry: "callDeepInner", options: "--compiler-enable-jit=false"

//! CHECKER      Call method of object nested member JIT (slow)
//! RUN          force_jit: true, entry: "callDeepInner", options: "--compiler-regex=qualified_call.ETSGLOBAL::callDeepInner --compiler-enable-fast-interop=false"

//! CHECKER      Call method of object nested member JIT (fast)
//! RUN          force_jit: true, entry: "callDeepInner", options: "--compiler-regex=qualified_call.ETSGLOBAL::callDeepInner --compiler-interop-try-single-scope=false"
//! METHOD       "callDeepInner"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerLoadJSConstantPool", 3
//! INST_COUNT   "Intrinsic.CompilerGetJSElement", 12
//! INST_COUNT   "Intrinsic.CompilerGetJSProperty", 15
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! PASS_AFTER_NEXT "GVN"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerLoadJSConstantPool", 1
//! # outerObj, inner, innerOuter, getInt, getDoubledInt
//! INST_COUNT   "Intrinsic.CompilerGetJSElement", 5
//! INST_COUNT   "Intrinsic.CompilerGetJSProperty", 15

//! CHECKER      Call method of object nested member AOT (slow)
//! RUN_AOT      options: "--compiler-regex=qualified_call.ETSGLOBAL::callDeepInner --compiler-enable-fast-interop=false"

//! CHECKER      Call method of object nested member AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=qualified_call.ETSGLOBAL::callDeepInner --compiler-interop-try-single-scope=false"
//! METHOD       "callDeepInner"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerLoadJSConstantPool", 3
//! INST_COUNT   "Intrinsic.CompilerGetJSElement", 12
//! INST_COUNT   "Intrinsic.CompilerGetJSProperty", 15
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! PASS_AFTER_NEXT "GVN"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerLoadJSConstantPool", 1
//! INST_COUNT   "Intrinsic.CompilerGetJSElement", 5
//! INST_COUNT   "Intrinsic.CompilerGetJSProperty", 15

function callDeepInner(): int {
    let res1 = outerObj.inner.innerOuter.inner.getInt() as int;
    let res2 = outerObj.inner.innerOuter.inner.getInt() as int;
    let res3 = outerObj.inner.innerOuter.inner.getDoubledInt() as int;
    if (res1 != 123) {
        return 1;
    }
    if (res2 != 123) {
        return 2;
    }
    if (res3 != 246) {
        return 3;
    }
    return 0;
}
