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

//! CHECKER         Select-instructions optimization by SelectOptimization, AOT
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*(equals|operands).*'"
//! RUN             entry: "select_optimization.ETSGLOBAL::main"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__equals_and_4"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 7
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_same_operands"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm", 1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_eq_costant_bool_operands_01X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 3
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_eq_costant_bool_operands_10X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 3
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_COUNT      "XorI", 1
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_ne_costant_bool_operands_10X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 1
//! INST_COUNT      "SelectImm NE", 2
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_ne_costant_bool_operands_01X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 1
//! INST_COUNT      "SelectImm NE", 2
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_COUNT      "XorI",1

//! CHECKER         Select-instructions optimization by SelectOptimization, JIT
//! RUN             force_jit: true, options: "--compiler-regex='.*(equals|operands).*'", entry: "select_optimization.ETSGLOBAL::main"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__equals_and_4"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 7
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_same_operands"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm", 1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_eq_costant_bool_operands_01X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 3
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_eq_costant_bool_operands_10X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 3
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_COUNT      "XorI", 1
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_ne_costant_bool_operands_10X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 1
//! INST_COUNT      "SelectImm NE", 2
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//!
//! METHOD          "select_optimization.ETSGLOBAL::__noinline__select_ne_costant_bool_operands_01X0"
//! PASS_AFTER      "MemoryCoalescing"
//! INST_NOT        "Select"
//! INST_NOT        "XorI"
//! PASS_AFTER      "SelectOptimization"
//! INST_COUNT      "SelectImm EQ", 1
//! INST_COUNT      "SelectImm NE", 2
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "Select"
//! INST_COUNT      "XorI",1

function __noinline__equals_and_4(a: Object, b: Object) {
    return a == b && b == a && a == b && b == a;
}

function __noinline__select_same_operands(x: int) {
    if ((x & 2) != 0) {
        x += 1;
    }
    else {
        x += 1;
    }
    return x;
}

function __noinline__select_eq_costant_bool_operands_01X0(a: Object, b: Object) {
    let result = false;
    if (a == b) {
        result = a == b ? true : false;
    }
    else {
        result = a == b ? true : false;
    }
    return result;
}

function __noinline__select_eq_costant_bool_operands_10X0(a: Object, b: Object) {
    let result = false;
    if (a == b) {
        result = a == b ? false : true;
    }
    else {
        result = a == b ? false : true;
    }
    return result;
}

function __noinline__select_ne_costant_bool_operands_10X0(a: Object, b: Object) {
    let result = false;
    if (a != b) {
        result = a == b ? true : false;
    }
    else {
        result = a == b ? true : false;
    }
    return result;
}

function __noinline__select_ne_costant_bool_operands_01X0(a: Object, b: Object) {
    let result = false;
    if (a != b) {
        result = a == b ? false : true;
    }
    else {
        result = a == b ? false : true;
    }
    return result;
}

function main(): void {
    let a = new Object;
    let b = new Object;

    arktest.assertEQ(__noinline__equals_and_4(a, a), true);
    arktest.assertEQ(__noinline__equals_and_4(a, b), false);

    arktest.assertEQ(__noinline__select_same_operands(1), 2);
    arktest.assertEQ(__noinline__select_same_operands(2), 3);

    arktest.assertEQ(__noinline__select_eq_costant_bool_operands_01X0(a, a), true);
    arktest.assertEQ(__noinline__select_eq_costant_bool_operands_01X0(a, b), false);

    arktest.assertEQ(__noinline__select_eq_costant_bool_operands_10X0(a, a), false);
    arktest.assertEQ(__noinline__select_eq_costant_bool_operands_10X0(a, b), true);

    arktest.assertEQ(__noinline__select_ne_costant_bool_operands_10X0(a, a), true);
    arktest.assertEQ(__noinline__select_ne_costant_bool_operands_10X0(a, b), false);

    arktest.assertEQ(__noinline__select_ne_costant_bool_operands_01X0(a, a), false);
    arktest.assertEQ(__noinline__select_ne_costant_bool_operands_01X0(a, b), true);
}