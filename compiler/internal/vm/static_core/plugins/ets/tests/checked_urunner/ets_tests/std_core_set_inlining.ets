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

//! CHECKER       AOT + external inlining: inline Map methods inside Set
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*test_set_.*' --compiler-inline-external-methods-aot=true", entry: "std_core_set_inlining.ETSGLOBAL::main"
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_bool,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_bool,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_bool"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_char,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_char,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_char"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_byte,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_byte,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_byte"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_short,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_short,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_short"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_int,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_int,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_int"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! RUN           entry: "std_core_set_inlining.ETSGLOBAL::main"

//! CHECKER       JIT: inline Map methods inside Set
//! RUN           force_jit: true, options: "--compiler-regex='.*test_set_.*'", entry: "std_core_set_inlining.ETSGLOBAL::main"
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_bool,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_bool,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_bool"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_char,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_char,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_char"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_byte,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_byte,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_byte"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_short,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_short,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_short"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_int,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining\.ETSGLOBAL::test_set_int,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining.ETSGLOBAL::test_set_int"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/

function test_set_bool() {
    let set: Set<boolean> = new Set<boolean>
    set.add(false)
    arktest.assertTrue(set.has(false))
    arktest.assertFalse(set.has(true))

    set.add(true)
    arktest.assertTrue(set.delete(true))
    arktest.assertFalse(set.delete(true))
}

function test_set_char() {
    let set: Set<char> = new Set<char>
    set.add(c'A')
    arktest.assertTrue(set.has(c'A'))
    arktest.assertFalse(set.has(c'B'))

    set.add(c'B')
    arktest.assertTrue(set.delete(c'B'))
    arktest.assertFalse(set.delete(c'B'))

    set.add(c'\u012C')
    arktest.assertTrue(set.has(c'\u012C'))
    arktest.assertFalse(set.has(c'\u002C'))
    arktest.assertFalse(set.has(c'\u0001'))
}

function test_set_byte() {
    let set: Set<byte> = new Set<byte>
    set.add((1).toByte())
    arktest.assertTrue(set.has((1).toByte()))
    arktest.assertFalse(set.has((2).toByte()))

    set.add((2).toByte())
    arktest.assertTrue(set.delete((2).toByte()))
    arktest.assertFalse(set.delete((2).toByte()))
}

function test_set_short() {
    let set: Set<short> = new Set<short>
    set.add((1).toShort())
    arktest.assertTrue(set.has((1).toShort()))
    arktest.assertFalse(set.has((2).toShort()))

    set.add((2).toShort())
    arktest.assertTrue(set.delete((2).toShort()))
    arktest.assertFalse(set.delete((2).toShort()))
}

function test_set_int(): void {
    let set: Set<int> = new Set<int>
    set.add(1)
    arktest.assertTrue(set.has(1))
    arktest.assertFalse(set.has(2))

    set.add(2)
    arktest.assertTrue(set.delete(2))
    arktest.assertFalse(set.delete(2))
}

function main(): void {
    test_set_bool()
    test_set_char()
    test_set_byte()
    test_set_short()
    test_set_int()
}
