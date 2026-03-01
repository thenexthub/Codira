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

// NOTE: using `undefined` directly gives "Failed to emit binary data: Argument 3 of function $jscall.invoke has undefined type" (#14501)
import { cons, car, cdr, vundefined }
    from "checked/js_call/js_call.js"

//! CHECKER      Tree of js function calls JIT
//! RUN          force_jit: true, entry: "test_call_tree", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::call_tree --compiler-interop-try-single-scope=false"
//! METHOD       "call_tree"
//! PASS_AFTER   "IrBuilder"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 4
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 4
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 4
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 4
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 4
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 3
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3 # remove dominated wrap of x
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 1 # only conversion of  LoadStatic $dynmodule... left
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 3 # cannot remove because we return JSValue
//! INST_NOT     "Intrinsic.CompilerConvertRefTypeToLocal"


function call_tree(x: int): JSValue {
    let list = cons(x, -1);
    if (x < 10) {
        list = cons(1, list);
    } else {
        if (x < 20) {
            list = cons(x, list);
        } else {
            list = cdr(list);
        }
    }
    return list;
}

function test_call_tree(): int {
    if (car(call_tree(5)) as int != 1) {
        return 1;
    }
    if (car(call_tree(15)) as int != 15) {
        return 2;
    }
    if (call_tree(25) as int != -1) {
        return 3;
    }
    return 0;
}

//! CHECKER      JS function calls in tree leaves, create common scope start JIT
//! RUN          force_jit: true, entry: "test_call_hoist_scope_start", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::call_hoist_scope_start --compiler-interop-try-single-scope=false"
//! METHOD       "call_hoist_scope_start"
//! PASS_AFTER   "IrBuilder"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3

function call_hoist_scope_start(x: int): JSValue {
    let list = vundefined;
    if (x < 10) {
        list = cons(1, vundefined);
    } else {
        if (x < 20) {
            list = cons(x, vundefined);
        } else {
            list = cons(x * 2, vundefined);
        }
    }
    return list;
}

function test_call_hoist_scope_start(): int {
    if (car(call_hoist_scope_start(5)) as int != 1) {
        return 1;
    }
    if (car(call_hoist_scope_start(15)) as int != 15) {
        return 2;
    }
    if (car(call_hoist_scope_start(25)) as int != 50) {
        return 3;
    }
    return 0;
}

//! CHECKER      ETS loop (not countable) between JS function calls
//! RUN          force_jit: true, entry: "loop_into_scope", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::loop_into_scope --compiler-interop-try-single-scope=false"
//! METHOD       "loop_into_scope"
//! PASS_AFTER   "IrBuilder"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 3
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 3
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1 # conversion of JSRuntimeGetPropertyJSValue to local, NOTE(aefremov): remove
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToF64", 1
//! INST_NOT     "Intrinsic.CompilerConvertLocalToJSValue"

function loop_into_scope(): int {
    let list = cons(1, vundefined);
    let x = 0;
    for (let i = 1; i < 10; i *= 2) {
        x += i;
    }
    list = cons(x, list);
    if (car(list) as int != 15) {
        return 1;
    }
    return 0;
}

//! CHECKER      Fully unrolled and not unrolled countable loops with JS calls
//! RUN          force_jit: true, entry: "small_loop_with_scopes", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::small_loop_with_scopes --compiler-interop-try-single-scope=false"
//! METHOD       "small_loop_with_scopes"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 4
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 4
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! # Additional scopes were created in LoopUnroll
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 8
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 8
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 8
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 8
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 8
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 3
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 3
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 3
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 2
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 3
//! INST         "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    "prop: head"
//! INST_NEXT    "Intrinsic.CompilerCreateLocalScope"
//! INST_NEXT    "Intrinsic.CompilerCreateLocalScope"

function small_loop_with_scopes(): int {
    let list = cons(1, vundefined);
    // loop removed completely in unroll
    for (let i = 1; i <= 5; i++) {
        list = cons(i, list);
    }
    // loop not unrolled; two scopes are left inside loop because of side exit
    for (let i = 5; i >= 1; i--) {
        if (car(list) as int != i) {
            return 1;
        }
        list = cdr(list);
    }
    return 0;
}

//! CHECKER      Countable loop with JS calls moved into one scope
//! RUN          force_jit: true, entry: "large_loop_with_scopes", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::large_loop_with_scopes --compiler-loop-unroll=false --compiler-interop-try-single-scope=false"
//! METHOD       "large_loop_with_scopes"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 5
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 5
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 5
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 5
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 5
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 3
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 3

function large_loop_with_scopes(): int {
    // NOTE(aefremov): scope start won't be hoisted from loop in current implementation, so we need a scope before loop in this test
    let list = cons(0, vundefined);
    for (let i = 1; i <= 9; i++) {
        list = cons(i, list);
    }
    let res = 0;
    for (let i = 9; i >= 1; i--) {
        if (car(list) as int != i) {
            res = 1;
        }
        list = cdr(list);
    }
    // manually peeled last iteration, see note above
    if (car(list) as int != 0) {
        res = 1;
    }
    return res;
}

//! CHECKER      ETS if chain between JS function calls
//! RUN          force_jit: true, entry: "test_if_chain_into_scope", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::if_chain_into_scope --compiler-interop-try-single-scope=false"
//! METHOD       "if_chain_into_scope"
//! PASS_AFTER   "IrBuilder"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "IfImm", 2
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 2
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 2
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 2
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 2
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 2
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 1
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 1

function if_chain_into_scope(x: int): JSValue {
    let list = cons(1, vundefined);
    if (x < 0) {
        x = -x;
    }
    if (x > 10) {
        x = 10;
    }
    return cons(x, list);
}

function test_if_chain_into_scope(): int {
    let x = car(if_chain_into_scope(-2)) as int;
    if (x != 2) {
        return 1;
    }
    x = car(if_chain_into_scope(15)) as int;
    if (x != 10) {
        return 2;
    }
    return 0;
}

//! CHECKER      ETS if chain with JS function calls
//! RUN          force_jit: true, entry: "test_if_chain_with_scopes", options: "--compiler-regex=merge_local_scopes.ETSGLOBAL::if_chain_with_scopes --compiler-interop-try-single-scope=false"
//! METHOD       "if_chain_with_scopes"
//! PASS_AFTER   "IrBuilder"
//! PASS_BEFORE  "InteropIntrinsicOptimization"
//! INST_COUNT   "IfImm", 2
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 5
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 5
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 5
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 5
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 5
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 5
//! PASS_AFTER   "InteropIntrinsicOptimization"
//! INST_COUNT   "Intrinsic.CompilerCreateLocalScope", 1
//! INST_COUNT   "Intrinsic.CompilerDestroyLocalScope", 1
//! PASS_AFTER   "DeoptimizeElimination"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT   "Intrinsic.CompilerConvertI32ToLocal", 3 # duplicate wraps of x removed
//! INST_COUNT   "Intrinsic.CompilerConvertJSValueToLocal", 1
//! INST_COUNT   "Intrinsic.CompilerConvertLocalToJSValue", 5
//! INST_COUNT   "Intrinsic.CompilerConvertRefTypeToLocal", 4

function if_chain_with_scopes(x: int): JSValue {
    let list = cons(1, vundefined);
    if (x < 0) {
        list = cons(x, list);
    }
    if (x < -10) {
        list = cons(-10, list);
    } else {
        list = cons(x, list);
    }
    return cons(x, list);
}

function compare_lists(list: JSValue, expected: FixedArray<int>): boolean {
    for (let i: int of expected) {
        if (car(list) as int != i) {
            return false;
        }
        list = cdr(list);
    }
    return true;
}

function test_if_chain_with_scopes(): int {
    let list = if_chain_with_scopes(2);
    if (!compare_lists(list, [2, 2, 1])) {
        return 1;
    }
    list = if_chain_with_scopes(-12);
    if (!compare_lists(list, [-12, -10, -12, 1])) {
        return 2;
    }
    return 0;
}
