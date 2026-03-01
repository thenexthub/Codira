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
//! RUN_PAOC      options: "--compiler-regex='.*test_set_.*' --compiler-inline-external-methods-aot=true", entry: "std_core_set_inlining_part2.ETSGLOBAL::main"
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_long,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_long,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_long"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_float,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_float,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_float"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_double,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_double,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_double"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_string,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_string,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_string"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! RUN           entry: "std_core_set_inlining_part2.ETSGLOBAL::main"

//! CHECKER       JIT: inline Map methods inside Set
//! RUN           force_jit: true, options: "--compiler-regex='.*test_set_.*'", entry: "std_core_set_inlining_part2.ETSGLOBAL::main"
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_long,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_long,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_long"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_float,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_float,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_float"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_double,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_double,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_double"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/
//!
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_string,std\.core\.Set::has,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std_core_set_inlining_part2\.ETSGLOBAL::test_set_string,std\.core\.Set::delete,.*,STATIC,SUCCESS/
//! EVENT         /Inline,std\.core\.Set::has,std\.core\.Map::has,.*,(SUCCESS|DEVIRTUALIZED)/
//! EVENT         /Inline,std\.core\.Set::delete,std\.core\.Map::delete,.*,(SUCCESS|DEVIRTUALIZED)/
//! METHOD        "std_core_set_inlining_part2.ETSGLOBAL::test_set_string"
//! PASS_AFTER    "IrBuilder"
//! INST          /CallStatic.*std\.core\.Set::has/
//! INST          /CallStatic.*std\.core\.Set::delete/
//! PASS_AFTER    "Inline"
//! INST          /Call.*std\.core\.Set::has/
//! INST          /Call.*std\.core\.Map::has/
//! INST          /Call.*std\.core\.Set::delete/
//! INST          /Call.*std\.core\.Map::delete/

function test_set_long() {
    let set: Set<long> = new Set<long>
    set.add(1)
    arktest.assertTrue(set.has(1))
    arktest.assertFalse(set.has(2))

    set.add(2)
    arktest.assertTrue(set.delete(2))
    arktest.assertFalse(set.delete(2))
}

function test_set_float() {
    let set: Set<float> = new Set<float>
    set.add(7.2f)
    arktest.assertTrue(set.has(7.2f))
    arktest.assertFalse(set.has(2))

    set.add(2.5f)
    arktest.assertTrue(set.delete(2.5f))
    arktest.assertFalse(set.delete(2.5f))
}

function test_set_double() {
    let set: Set<double> = new Set<double>
    set.add(1.5)
    arktest.assertTrue(set.has(1.5))
    arktest.assertFalse(set.has(2.1))

    set.add(2.1)
    arktest.assertTrue(set.delete(2.1))
    arktest.assertFalse(set.delete(2.1))
}

function test_set_string() {
    let set: Set<string> = new Set<string>
    arktest.assertFalse(set.has(""))
    set.add("")
    set.add("abc")
    set.add("defgh")
    arktest.assertTrue(set.has(""))
    arktest.assertTrue(set.has("abc"))
    arktest.assertTrue(set.has("defgh"))
    arktest.assertFalse(set.has("erty"))

    set.add("erty")
    arktest.assertTrue(set.delete("erty"))
    arktest.assertFalse(set.delete("erty"))
}

function main(): void {
    test_set_long()
    test_set_float()
    test_set_double()
    test_set_string()
}
