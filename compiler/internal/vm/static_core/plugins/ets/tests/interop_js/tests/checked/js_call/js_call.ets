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

import { cons, car, cdr, sum, TreeNode, makeDynObject, doNothing, makeSwappable, StaticClass, extractSquaredInt, ObjectWithPrototype, vundefined}
    from "checked/js_call/js_call.js"

//! CHECKER      Several js function calls JIT (slow)
//! RUN          force_jit: true, entry: "callSequence", options: "--compiler-regex=js_call.ETSGLOBAL::callSequence --compiler-enable-fast-interop=false"
//! METHOD       "callSequence"
//! PASS_AFTER   "IrBuilder"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"

//! CHECKER      Several js function calls JIT (fast)
//! RUN          force_jit: true, entry: "callSequence", options: "--compiler-regex=js_call.ETSGLOBAL::callSequence --compiler-interop-try-single-scope=false"
//! METHOD       "callSequence"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 3
//! INST_NOT     /LoadImmediate.*string/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 3
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 3
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 4
//! INST_COUNT   "Intrinsic.CompilerConvertF64ToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 3
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! PASS_BEFORE   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_NOT     "Intrinsic.CompilerConvertLocalToJSValue"
//! INST         "Intrinsic.CompilerConvertLocalToF64"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToF64"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToString"
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1

//! CHECKER      Several js function calls AOT (slow)
//! RUN_AOT      options: "--compiler-regex=js_call.ETSGLOBAL::callSequence --compiler-enable-fast-interop=false"
//! METHOD       "callSequence"
//! PASS_AFTER   "IrBuilder"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! RUN          entry: "callSequence"

//! CHECKER      Several js function calls AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::callSequence --compiler-interop-try-single-scope=false"
//! METHOD       "callSequence"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 3
//! INST_NOT     /LoadImmediate.*PandaFileOffset/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 3
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 3
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 4
//! INST_COUNT   "Intrinsic.CompilerConvertF64ToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 3
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    "Intrinsic.JSRuntimeGetValueString"
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! RUN          entry: "callSequence"

function callSequence(): int {
    for (let i = 0; i < 10; i++) {
        let sumInt = sum(i, 2) as int;
        let sumDouble = sum(i, 0.5) as double;
        let sumString = sum("abc", i) as string;
        if (sumInt != i + 2) {
            return 1;
        }
        if (abs(i + 0.5 - sumDouble) > 1e-6) {
            return 2;
        }
        if (sumString != "abc" + i) {
            return 3;
        }
    }
    return 0;
}

//! CHECKER      Chain of js function calls JIT (slow)
//! RUN          force_jit: true, entry: "callChain", options: "--compiler-regex=js_call.ETSGLOBAL::callChain --compiler-enable-fast-interop=false"
//! METHOD       "callChain"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/

//! CHECKER      Chain of js function calls JIT (fast)
//! RUN          force_jit: true, entry: "callChain", options: "--compiler-regex=js_call.ETSGLOBAL::callChain --compiler-interop-try-single-scope=false"
//! METHOD       "callChain"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 6
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 6
//! INST_NOT     /LoadImmediate.*string/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 6
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 6
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 6
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 6
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 6
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 6
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! PASS_BEFORE   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 6
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 1
//! INST_NOT     "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3
//! # Intrinsic.CompilerConvertRefTypeToLocal is user of Intrinsic.JSRuntimeGetPropertyJSValue
//! # TODO(aefremov): find a way to remove
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToF64", 1

//! CHECKER      Chain of js function calls AOT (slow)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::callChain --compiler-enable-fast-interop=false"
//! METHOD       "callChain"
//! PASS_AFTER   "Codegen"
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! RUN          entry: "callChain"

//! CHECKER      Chain of js function calls LLVMAOT (slow)
//! RUN_LLVM     options: "--compiler-regex=js_call.ETSGLOBAL::callChain --compiler-enable-fast-interop=false"
//! RUN          entry: "callChain"

//! CHECKER      Chain of js function calls AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::callChain --compiler-interop-try-single-scope=false"
//! METHOD       "callChain"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 6
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 6
//! INST_NOT     /LoadImmediate.*PandaFileOffset/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 6
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 6
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 6
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 6
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 6
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 6
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! INST_NEXT    /i32.*Cast f64/
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! RUN          entry: "callChain"

function callChain(): int {
    for (let i = 0; i < 10; i++) {
        let list = cons(0, cons(1, cons(i, vundefined)));
        let tail = car(cdr(cdr(list)));
        if (tail as int != i) {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Chain of js new calls JIT (slow)
//! RUN          force_jit: true, entry: "newChain", options: "--compiler-regex=js_call.ETSGLOBAL::newChain --compiler-enable-fast-interop=false"
//! METHOD       "newChain"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetValueDouble", 1
//! INST         /i32.*Cast f64/

//! CHECKER      Chain of js new calls JIT (fast)
//! RUN          force_jit: true, entry: "newChain", options: "--compiler-regex=js_call.ETSGLOBAL::newChain --compiler-interop-try-single-scope=false"
//! METHOD       "newChain"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 4
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 4
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 4
//! INST_NOT     /LoadImmediate.*string/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 4
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 4
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 2
//! INST_COUNT   "Intrinsic.CompilerJSNewInstance", 3
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 1
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 4
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 4
//! INST_COUNT   "Intrinsic.JSRuntimeGetValueDouble", 1
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 10
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 10
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 10
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 9
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 2
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 7
//! # We convert Phi inputs to JSValue and then convert Phi back to value in local scope, TODO(aefremov): remove
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 2
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToF64", 1

//! CHECKER      Chain of js new calls AOT (slow)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::newChain --compiler-enable-fast-interop=false"
//! METHOD       "newChain"
//! PASS_AFTER   "Codegen"
//! INST_COUNT   "Intrinsic.JSRuntimeGetValueDouble", 1
//! RUN          entry: "newChain"

//! CHECKER      Chain of js new calls LLVMAOT (slow)
//! RUN_LLVM     options: "--compiler-regex=js_call.ETSGLOBAL::newChain --compiler-enable-fast-interop=false"
//! RUN          entry: "newChain"

//! CHECKER      Chain of js new calls AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::newChain --compiler-interop-try-single-scope=false"
//! METHOD       "newChain"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 4
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 4
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 4
//! INST_NOT     /LoadImmediate.*PandaFileOffset/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 4
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 4
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 2
//! INST_COUNT   "Intrinsic.CompilerJSNewInstance", 3
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 1
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 4
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 4
//! INST_COUNT   "Intrinsic.JSRuntimeGetValueDouble", 1
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! RUN          entry: "newChain"

function newChain(): int {
    let last = new TreeNode(0);
    for (let i = 0; i < 10; i++) {
        let right = new TreeNode(100);
        let current = new TreeNode(i, last, right);
        last = current;
    }
    let jsTreeSum = last.sum() as int;
    if (jsTreeSum != 1045) {
        return 1;
    }
    return 0;
}

//! CHECKER      JS load and store JIT (slow)
//! RUN          force_jit: true, entry: "loadStoreChain", options: "--compiler-regex=js_call.ETSGLOBAL::loadStoreChain --compiler-enable-fast-interop=false"
//! METHOD       "loadStoreChain"
//! PASS_AFTER   "IrBuilder"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.CompilerEtsStrictEquals"

//! CHECKER      JS load and store JIT (fast)
//! RUN          force_jit: true, entry: "loadStoreChain", options: "--compiler-regex=js_call.ETSGLOBAL::loadStoreChain"
//! METHOD       "loadStoreChain"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT    "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT     /LoadImmediate.*string/
//! INST_NEXT    "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT    "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT    "Intrinsic.CompilerConvertI32ToLocal"
//! INST_NEXT    "Intrinsic.CompilerJSNewInstance"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT    "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.CompilerEtsStrictEquals"

//! CHECKER      JS load and store AOT (slow)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::loadStoreChain --compiler-enable-fast-interop=false"
//! METHOD       "loadStoreChain"
//! PASS_AFTER   "Codegen"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.CompilerEtsStrictEquals"
//! RUN          entry: "loadStoreChain"

//! CHECKER      JS load and store LLVMAOT (slow)
//! RUN_AOT      options: "--compiler-regex=js_call.ETSGLOBAL::loadStoreChain --compiler-enable-fast-interop=false"
//! RUN          entry: "loadStoreChain"

//! CHECKER      JS load and store AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::loadStoreChain"
//! METHOD       "loadStoreChain"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT    "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT     /LoadImmediate.*PandaFileOffset/
//! INST_NEXT    "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT    "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT    "Intrinsic.CompilerConvertI32ToLocal"
//! INST_NEXT    "Intrinsic.CompilerJSNewInstance"
//! INST_NEXT    "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT    "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT    "Intrinsic.CompilerEtsStrictEquals"
//! RUN          entry: "loadStoreChain"

function loadStoreChain(): int {
    for (let i = 0; i < 10; i++) {
        let obj = new makeDynObject(i);
        obj.v1 = obj.v0;
        obj.v2 = obj.v1;
        if (obj.v2 !== obj.v0) {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Define object method in js and call it from ets, JIT (slow)
//! RUN          force_jit: true, entry: "methodCall", options: "--compiler-regex=js_call.ETSGLOBAL::methodCall --compiler-enable-fast-interop=false"
//! METHOD       "methodCall"
//! PASS_AFTER   "IrBuilder"

//! CHECKER      Define object method in js and call it from ets, JIT (fast)
//! RUN          force_jit: true, entry: "methodCall", options: "--compiler-regex=js_call.ETSGLOBAL::methodCall --compiler-interop-try-single-scope=false"
//! METHOD       "methodCall"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 6
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 6
//! INST_NOT     /LoadImmediate.*string/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 6
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 6
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerJSNewInstance", 1
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 5
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 6
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 6
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4
//! PASS_BEFORE   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 6
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 2
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 2

//! CHECKER      Define object method in js and call it from ets, AOT (slow)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::methodCall --compiler-enable-fast-interop=false"
//! METHOD       "methodCall"
//! PASS_AFTER   "IrBuilder"
//! RUN          entry: "methodCall"

//! CHECKER      Define object method in js and call it from ets, LLVMAOT (slow)
//! RUN_LLVM     options: "--compiler-regex=js_call.ETSGLOBAL::methodCall --compiler-enable-fast-interop=false"
//! RUN          entry: "methodCall"

//! CHECKER      Define object method in js and call it from ets, AOT (fast)
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::methodCall"
//! METHOD       "methodCall"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 6
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 6
//! INST_COUNT   "Intrinsic.CompilerResolveQualifiedJSCall", 6
//! INST_NOT     /LoadImmediate.*PandaFileOffset/
//! INST_COUNT   "Intrinsic.CompilerLoadResolvedJSFunction", 6
//! INST_COUNT   "Intrinsic.CompilerJSCallCheck", 6
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerJSNewInstance", 1
//! INST_COUNT   "Intrinsic.CompilerJSCallFunction", 5
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 6
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 6
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4
//! RUN          entry: "methodCall"

function methodCall(): int {
    for (let i = 0; i < 5; i++) {
        let pair = new doNothing();
        pair.first = -i;
        pair.second = i;
        makeSwappable(pair);
        pair.swap();
        // TODO (aefremov): we do not extend handle scopes downwards,
        // so we get 2 scopes instead of 1 here because of return
        if (pair.first as int != i || pair.second as int != -i) {
            return 1;
        }
        pair.swap();
        pair.swap();
        pair.swap();
        if (pair.first as int != -i || pair.second as int != i) {
            return 2;
        }
    }
    return 0;
}

//! CHECKER       Call JS static method and access static property, JIT (slow)
//! RUN           force_jit: true, entry: "staticMethodCall", options: "--compiler-regex=js_call.ETSGLOBAL::staticMethodCall --compiler-enable-fast-interop=false"
//! METHOD        "staticMethodCall"
//! PASS_AFTER    "IrBuilder"
//! INST          "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for lhs
//! INST_NEXT     "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for rhs
//! INST_NEXT_NOT "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT     "Intrinsic.JSRuntimeGetPropertyDouble" # get StaticClass.staticProperty
//! INST_NEXT     "Intrinsic.JSRuntimeSetPropertyDouble" # set StaticClass.staticProperty
//! INST_NEXT     "Intrinsic.JSRuntimeGetValueDouble"
//! INST_COUNT      "Intrinsic.JSRuntimeGetValueDouble", 1

//! CHECKER         Call JS static method and access static property, JIT (fast)
//! RUN             force_jit: true, entry: "staticMethodCall", options: "--compiler-regex=js_call.ETSGLOBAL::staticMethodCall"
//! METHOD          "staticMethodCall"
//! PASS_AFTER      "IrBuilder"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for lhs
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for rhs
//! INST_NEXT_NOT   "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble" # get StaticClass.staticProperty
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyDouble" # set StaticClass.staticProperty
//! INST_NEXT       "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT       "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT       "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT        /LoadImmediate.*string/
//! INST_NEXT       "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT       "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT       "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT       "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT       "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT       "Intrinsic.JSRuntimeGetValueDouble"

//! CHECKER         Call JS static method and access static property, AOT (slow)
//! RUN_PAOC        options: "--compiler-regex=js_call.ETSGLOBAL::staticMethodCall --compiler-enable-fast-interop=false"
//! METHOD          "staticMethodCall"
//! PASS_AFTER      "IrBuilder"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for lhs
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for rhs
//! INST_NEXT_NOT   "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble" # get StaticClass.staticProperty
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyDouble" # set StaticClass.staticProperty
//! INST_NEXT       "Intrinsic.JSRuntimeGetValueDouble"
//! INST_COUNT      "Intrinsic.JSRuntimeGetValueDouble", 1
//! RUN             entry: "staticMethodCall"

//! CHECKER      Call JS static method and access static property, LLVMAOT (slow)
//! RUN_LLVM     options: "--compiler-regex=js_call.ETSGLOBAL::staticMethodCall --compiler-enable-fast-interop=false"
//! RUN          entry: "staticMethodCall"

//! CHECKER         Call JS static method and access static property, AOT (fast)
//! RUN_PAOC        options: "--compiler-regex=js_call.ETSGLOBAL::staticMethodCall"
//! METHOD          "staticMethodCall"
//! PASS_AFTER      "IrBuilder"
//! INST            "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for lhs
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyJSValue" # get StaticClass for rhs
//! INST_NEXT_NOT   "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NEXT       "Intrinsic.JSRuntimeGetPropertyDouble" # get StaticClass.staticProperty
//! INST_NEXT       "Intrinsic.JSRuntimeSetPropertyDouble" # set StaticClass.staticProperty
//! INST_NEXT       "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT       "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT       "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT        /LoadImmediate.*PandaFileOffset/
//! INST_NEXT       "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT       "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT       "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT       "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT       "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT       "Intrinsic.JSRuntimeGetValueDouble"
//! RUN             entry: "staticMethodCall"

function staticMethodCall(): int {
    for (let i = 1; i <= 5; i++) {
        StaticClass.staticProperty = StaticClass.staticProperty as int + 1;
        if (StaticClass.staticMethod() as int != 110 + i) {
            return 1;
        }
    }
    return 0;
}

class EtsClass {
    intValue: int;
    constructor(value: int) {
        this.intValue = value;
    }
}

//! CHECKER      Use ets object from js, JIT (slow)
//! RUN          force_jit: true, entry: "jscallEtsObject", options: "--compiler-regex=js_call.ETSGLOBAL::jscallEtsObject --compiler-enable-fast-interop=false"
//! METHOD       "jscallEtsObject"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueDouble"

//! CHECKER     Use ets object from js, JIT (fast)
//! RUN         force_jit: true, entry: "jscallEtsObject", options: "--compiler-regex=js_call.ETSGLOBAL::jscallEtsObject"
//! METHOD     "jscallEtsObject"
//! PASS_AFTER "IrBuilder"
//! INST       "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT  "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT  "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT   /LoadImmediate.*string/
//! INST_NEXT  "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT  "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT  "Intrinsic.CompilerConvertRefTypeToLocal"
//! INST_NEXT  "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT  "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT  "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT  "Intrinsic.JSRuntimeGetValueDouble"

//! CHECKER      Use ets object from js, AOT (slow)
//! RUN_AOT      options: "--compiler-regex=js_call.ETSGLOBAL::jscallEtsObject --compiler-enable-fast-interop=false"
//! METHOD       "jscallEtsObject"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueDouble"
//! RUN          entry: "jscallEtsObject"

//! CHECKER     Use ets object from js, AOT (fast)
//! RUN_PAOC    options: "--compiler-regex=js_call.ETSGLOBAL::jscallEtsObject"
//! METHOD     "jscallEtsObject"
//! PASS_AFTER "IrBuilder"
//! INST       "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT  "Intrinsic.CompilerConvertJSValueToLocal"
//! INST_NEXT  "Intrinsic.CompilerResolveQualifiedJSCall"
//! INST_NOT   /LoadImmediate.*PandaFileOffset/
//! INST_NEXT  "Intrinsic.CompilerLoadResolvedJSFunction"
//! INST_NEXT  "Intrinsic.CompilerJSCallCheck"
//! INST_NEXT  "Intrinsic.CompilerConvertRefTypeToLocal"
//! INST_NEXT  "Intrinsic.CompilerJSCallFunction"
//! INST_NEXT  "Intrinsic.CompilerConvertLocalToJSValue"
//! INST_NEXT  "Intrinsic.CompilerDestroyLocalScope"
//! INST_NEXT  "Intrinsic.JSRuntimeGetValueDouble"
//! RUN        entry: "jscallEtsObject"

function jscallEtsObject(): int {
    for (let i = 1; i <= 5; i++) {
        let etsObject = new EtsClass(i);
        let res = extractSquaredInt(etsObject) as int;
        if (res != i * i) {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Load value from js prototype, JIT
//! RUN          force_jit: true, entry: "jsLoadFromPrototype", options: "--compiler-regex=js_call.ETSGLOBAL::jsLoadFromPrototype"
//! METHOD       "jsLoadFromPrototype"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetPropertyDouble"

//! CHECKER      Load value from js prototype, AOT
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::jsLoadFromPrototype"
//! METHOD       "jsLoadFromPrototype"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetPropertyDouble"
//! RUN          entry: "jsLoadFromPrototype"

function jsLoadFromPrototype(): int {
    for (let i = 1; i <= 5; i++) {
        let obj = new ObjectWithPrototype();
        if (obj.prototypeValue as int != 5) {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Load value overridden in js object, JIT
//! RUN          force_jit: true, entry: "jsLoadOverridden", options: "--compiler-regex=js_call.ETSGLOBAL::jsLoadOverridden"
//! METHOD       "jsLoadOverridden"
//! PASS_AFTER   "IrBuilder"
//! INST        "Intrinsic.JSRuntimeGetPropertyDouble"

//! CHECKER      Load value overridden in js object, AOT
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::jsLoadOverridden"
//! METHOD       "jsLoadOverridden"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetPropertyDouble"
//! RUN          entry: "jsLoadOverridden"

function jsLoadOverridden(): int {
    for (let i = 1; i <= 5; i++) {
        let obj = new ObjectWithPrototype();
        if (obj.overriddenValue as int != 4) {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Call function from js prototype, JIT
//! RUN          force_jit: true, entry: "jsCallFromPrototype", options: "--compiler-regex=js_call.ETSGLOBAL::jsCallFromPrototype"
//! METHOD       "jsCallFromPrototype"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueString"

//! CHECKER      Call function from js prototype, AOT
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::jsCallFromPrototype"
//! METHOD       "jsCallFromPrototype"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueString"
//! RUN          entry: "jsCallFromPrototype"

function jsCallFromPrototype(): int {
    for (let i = 1; i <= 5; i++) {
        let obj = new ObjectWithPrototype();
        if (obj.prototypeFunction() as String != "prototype function") {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Call function overridden in js object, JIT
//! RUN          force_jit: true, entry: "jscallOverridden", options: "--compiler-regex=js_call.ETSGLOBAL::jscallOverridden"
//! METHOD       "jscallOverridden"
//! PASS_AFTER   "IrBuilder"

//! CHECKER      Call function overridden in js object, AOT
//! RUN_PAOC     options: "--compiler-regex=js_call.ETSGLOBAL::jscallOverridden"
//! METHOD       "jscallOverridden"
//! RUN          entry: "jscallOverridden"

function jscallOverridden(): int {
    for (let i = 1; i <= 5; i++) {
        let obj = new ObjectWithPrototype();
        if (obj.overriddenFunction() as String != "overridden") {
            return 1;
        }
    }
    return 0;
}

//! CHECKER      Check that references stay valid after rehash in String->JSValue cast implementation
//! RUN          force_jit: true, entry: "testRehash", options: "--compiler-regex=js_call.ETSGLOBAL::testRehash"

function testRehash(): int {
    let count = 40
    let jsValues: FixedArray<JSValue> = new FixedArray<JSValue>(count)
    for (let i = 0; i < count; i++) {
        jsValues[i] = (i as Int).toString() as JSValue
    }
    let jsValues2: FixedArray<JSValue> = new FixedArray<JSValue>(count)
    for (let i = 0; i < count; i++) {
        jsValues2[i] = (i as Int).toString() as JSValue
    }
    for (let i = 0; i < count; i++) {
        if (jsValues[i] as String != (i as Int).toString()) {
            return 1
        }
        if (jsValues2[i] as String != (i as Int).toString()) {
            return 2
        }
    }
    return 0
}
