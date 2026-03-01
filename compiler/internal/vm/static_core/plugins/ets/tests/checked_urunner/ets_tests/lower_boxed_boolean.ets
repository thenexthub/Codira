/*
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

//! CHECKER       (JIT) Check LowerBoxedBoolean pass
//! RUN           force_jit: true, options: "--compiler-regex='.*lower_boxed_boolean.*test.*' ", entry: "lower_boxed_boolean.ETSGLOBAL::main"
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test1"
//! PASS_AFTER    "Codegen"
//! INST_NOT      /LoadStatic.*std.core.Boolean.TRUE/
//! INST_NOT      /LoadStatic.*std.core.Boolean.FALSE/
//! INST_NOT      /.*ref.*Phi.*/
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test2"
//! PASS_AFTER    "Codegen"
//! INST_NOT      /LoadStatic.*std.core.Boolean.TRUE/
//! INST_NOT      /LoadStatic.*std.core.Boolean.FALSE/
//! INST_NOT      /.*ref.*Phi.*/
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test3"
//! PASS_AFTER    "Codegen"
//! INST_NOT      /LoadStatic.*std.core.Boolean.TRUE/
//! INST_NOT      /LoadStatic.*std.core.Boolean.FALSE/
//! INST_NOT      /.*ref.*Phi.*v.*v.*v.*/
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test4"
//! PASS_BEFORE   "LowerBoxedBoolean"
//! INST          /LoadStatic.*CustomBoolean.TRUE/
//! INST          /LoadStatic.*CustomBoolean.FALSE/
//! INST          /.*ref.*Phi.*/
//! INST_NOT      /Constant.*0x1/
//! INST_NOT      /Constant.*0x0/
//! INST_NOT      /.*b.*Phi.*/
//! PASS_AFTER    "LowerBoxedBoolean"
//! INST          /LoadStatic.*CustomBoolean.TRUE/
//! INST          /LoadStatic.*CustomBoolean.FALSE/
//! INST          /.*ref.*Phi.*/
//! INST_NOT      /Constant.*0x1/
//! INST_NOT      /Constant.*0x0/
//! INST_NOT      /.*b.*Phi.*/
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test5"
//! INST          /.*ref.*Phi.*/
//! INST_NOT      /.*b.*Phi.*/
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test6"
//! PASS_BEFORE   "LowerBoxedBoolean"
//! INST_NOT      /.*b.*Phi.*/
//! PASS_AFTER    "LowerBoxedBoolean"
//! INST_NOT      /.*b.*Phi.*/
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test7"
//! PASS_BEFORE   "LowerBoxedBoolean"
//! INST_NOT      /.*ref.*Phi.*/
//! PASS_AFTER    "LowerBoxedBoolean"
//! PASS_AFTER    "Codegen"
//! INST_NOT      /LoadStatic.*std.core.Boolean.TRUE/

//! CHECKER       (AOT) Check LowerBoxedBoolean pass
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*lower_boxed_boolean.*test.*' --compiler-simplify-string-builder=false", entry: "lower_boxed_boolean.ETSGLOBAL::main"
//!
//! METHOD        "lower_boxed_boolean.ETSGLOBAL::test8"

// Check LowerBoxedBoolean optimization on Phi with two inputs.
function test1(input: boolean): boolean {
    let result: Boolean;
    if (input) {
        result = Boolean.TRUE;
    } else {
        result = Boolean.FALSE;
    }
    return result.valueOf();
}

// Check LowerBoxedBoolean optimization on Phi inside lambda.
function test2(array: Array<Number>): number {
    return array.findIndex((element: number) => {
        return element < -1;
    });
}

// Check LowerBoxedBoolean optimization on Phi with three inputs.
function test3(cond1: boolean, cond2: boolean): boolean {
    let b1: Boolean;
    if (cond1) {
        if (cond2) {
            b1 = Boolean.TRUE;
        } else {
            b1 = Boolean.FALSE;
        }
    } else {
        b1 = Boolean.TRUE;
    }
    return b1.valueOf();
}

// Check LowerBoxedBoolean doesn't optimize custom class fields named TRUE/FALSE.
class CustomBoolean {
    static TRUE: CustomBoolean = new CustomBoolean();
    static FALSE: CustomBoolean = new CustomBoolean();
}

function test4(input: boolean): CustomBoolean {
    let result: CustomBoolean;
    if (input) {
        result = CustomBoolean.TRUE;
    } else {
        result = CustomBoolean.FALSE;
    }
    return result;
}

// Check LowerBoxedBoolean doesn't optimize Phi inside a loop inside a lambda.
function test5(array: Array<Number>, v: number): Boolean {
    const f = () => {
        let result: Boolean = new Boolean(false);
        for (let i = 0; i < array.length; i++) {
            if (array[i] < v) {
                result = Boolean.TRUE;
            } else {
                result = Boolean.FALSE;
            }
        }
        return result;
    };
    return f();
}

// Check that LowerBoxedBoolean skips optimization due to a recursive Phi cycle.
function test6(array: Array<number>, value: number): boolean {
    let has_number: Boolean = Boolean.FALSE;
    for (let i = 0; i < array.length; i++) {
        if (array[i] == value) {
            if (!has_number) {
                has_number = Boolean.TRUE;
            }
        }
    }
    return has_number;
}

// Check LowerBoxedBoolean optimization on Compare with LoadStatic Boolean.TRUE as input.
function test7(): boolean {
    let b: Boolean | null = (true ? Boolean.TRUE : null);
    if (b === undefined) {
        return false;
    }
    return true;
}

function get_a_string(flag: Boolean = Boolean.FALSE): string {
    return "A";
}

// Check that LowerBoxedBoolean safely skips optimization for SaveState with no users (preventing segfault).
// CallStatic std.core.Boolean <- Phi <- SaveState <- None.
function test8(obj: Object, flag: boolean = false): string {
    let s = new StringBuilder("A");
    for (const i of Object.entries(obj)) {
        s.append(get_a_string(flag));
    }
    return s.toString();
}

function main(): int {
    let result1: boolean = test1(true);
    let array: Array<Number> = new Array<Number>(1, 2, 3, -2, 6, 7)
    let result2: number = test2(array);
    let result3: boolean = test3(true, false);
    let result4: CustomBoolean = test4(true);
    let result5: Boolean = test5(array, 5);
    let result6: Boolean = test6(array, 3);
    let result7: boolean = test7();
    let result8: string = test8("A");
    return 0;
}
