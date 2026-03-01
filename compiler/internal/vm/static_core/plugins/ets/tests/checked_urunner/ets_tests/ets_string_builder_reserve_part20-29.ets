/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       AOT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*reserve[0-9]*' --compiler-inlining=true"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_reserve20"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_if_reserve21"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_if_reserve22"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_loop_reserve23"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=19)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=19)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::types_reserve24"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=10)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=10)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve25"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",1
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",2
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve26"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",1
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",2
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve27"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve28"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::partially_uncountable_reserve29"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=6)"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=6)"
//! INST_NOT      "NewArray (size=9)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*reserve[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_reserve_part20-29.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_reserve20"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_if_reserve21"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_if_reserve22"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::multi_toString_loop_reserve23"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=19)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=19)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::types_reserve24"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=10)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=10)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve25"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",1
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",2
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve26"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",1
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_COUNT    "NewArray (size=3)",2
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve27"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::lenArray_reserve28"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part20-29.ETSGLOBAL::partially_uncountable_reserve29"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=6)"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=6)"
//! INST_NOT      "NewArray (size=9)"

function multi_toString_reserve20() : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    sb.append(sb.toString())
    sb.append(1)
    return sb.toString()
}

function multi_toString_if_reserve21() : string {
    let sb = new StringBuilder()            // applied
    sb.append(0)
    if (sb.toString().getLength() > 0)
        sb.append(sb.toString())
    sb.append(1)
    return sb.toString()
}

function multi_toString_if_reserve22() : string {
    let sb = new StringBuilder()            // applied
    sb.append(sb.toString())
    if (sb.toString().getLength() > 0)
        sb.append(sb.toString())
    sb.append(sb.toString())
    return sb.toString()
}

function multi_toString_loop_reserve23() : string {
    let sb = new StringBuilder()            // applied
    sb.append(sb.toString())
    for (let i = 0; i < 3; ++i) {
        sb.append(sb.toString())
        if (sb.toString().getLength() > 0)
            for (let j = 0; j < 3; ++j)
                sb.append(sb.toString())
        sb.append(sb.toString())
    }
    sb.append(sb.toString())
    if (sb.toString().getLength() > 0)
        sb.append(sb.toString())
    else
        sb.append(sb.toString())
    sb.append(sb.toString())
    return sb.toString()
}

function types_reserve24() : string {
    let sb = new StringBuilder()            // applied
    sb.append(false as boolean)
    sb.append(c'1' as char)
    sb.append(2 as byte)
    sb.append(3 as short)
    sb.append(4 as int)
    sb.append(5 as long)
    sb.append(6 as float)
    sb.append(7 as double)
    sb.append("8" as string)
    sb.append(new Object)
    return sb.toString()
}

function lenArray_reserve25() : string {
    let strings: FixedArray<string> = ["0", "1", "2"]
    let sb = new StringBuilder()            // applied, due loop is countable
    for (let i = 0; i < strings.length; ++i)
        sb.append(strings[i])
    return sb.toString()
}

function lenArray_reserve26() : string {
    let strings: FixedArray<string> = ["0", "1", "2"]
    let sb = new StringBuilder()            // applied, due loop is countable
    for (let s : string of strings)
        sb.append(s)
    return sb.toString()
}

function lenArray_reserve27(strings: FixedArray<string>) : string {
    let sb = new StringBuilder()            // not applied, due loop is uncountable
    for (let i = 0; i < strings.length; ++i)
        sb.append(strings[i])
    return sb.toString()
}

function lenArray_reserve28(strings: FixedArray<string>) : string {
    let sb = new StringBuilder()            // not applied, due loop is uncountable
    for (let s : string of strings)
        sb.append(s)
    return sb.toString()
}

function partially_uncountable_reserve29() : string {
    let sb_countable = new StringBuilder()  // applied, due loop is countable
    for (let i : int = 2; i < 8; ++i)
        sb_countable.append(i)

    let sb_uncountable = new StringBuilder()    // not applied, due loop is uncountable
    for (let i : number = 2; i < 11; ++i)
        sb_uncountable.append(i)

    return sb_countable.toString() + sb_uncountable.toString()
}

function main() : int {
    arktest.assertEQ(multi_toString_reserve20(), "001", "Wrong result at multi_toString_reserve20")
    arktest.assertEQ(multi_toString_if_reserve21(), "001", "Wrong result at multi_toString_if_reserve21")
    arktest.assertEQ(multi_toString_if_reserve22(), "", "Wrong result at multi_toString_if_reserve22")
    arktest.assertEQ(multi_toString_loop_reserve23(), "", "Wrong result at multi_toString_loop_reserve23")
    arktest.assertEQ(types_reserve24(), "false12345678[object Object]", "Wrong result at types_reserve24")
    arktest.assertEQ(lenArray_reserve25(), "012", "Wrong result at lenArray_reserve25")
    arktest.assertEQ(lenArray_reserve26(), "012", "Wrong result at lenArray_reserve26")
    arktest.assertEQ(lenArray_reserve27(["0", "1", "2"]), "012", "Wrong result at lenArray_reserve27")
    arktest.assertEQ(lenArray_reserve28(["0", "1", "2"]), "012", "Wrong result at lenArray_reserve28")
    arktest.assertEQ(partially_uncountable_reserve29(), "2345672345678910", "Wrong result at partially_uncountable_reserve29")

    return 0
}
