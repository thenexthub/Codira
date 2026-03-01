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
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::try_catch_reserve30"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::try_finally_reserve31"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::out_reserve32"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::in_reserve33"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::global_reserve34"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chain_reserve35"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::more_types_reserve36"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=20)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=20)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::nested_loop_reserve37"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=12)"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=12)"
//! INST          "NewArray (size=9)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::store_reserve39"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chain_reserve40"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chain_reserve41"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::arg_reserve42"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::store_reserve43"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::store_reserve44"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reserve
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*reserve[0-9]*' --compiler-inlining=true", entry: "ets_string_builder_reserve_part30-44.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::try_catch_reserve30"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::try_finally_reserve31"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::out_reserve32"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::in_reserve33"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::global_reserve34"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=1)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chain_reserve35"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::more_types_reserve36"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=20)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=20)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::nested_loop_reserve37"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=12)"
//! INST_NOT      "NewArray (size=9)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=12)"
//! INST          "NewArray (size=9)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::store_reserve39"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chain_reserve40"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::chain_reserve41"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=3)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST          "NewArray (size=3)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::arg_reserve42"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::store_reserve43"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//!
//! METHOD        "ets_string_builder_reserve_part30-44.ETSGLOBAL::store_reserve44"
//! PASS_BEFORE   "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"
//! PASS_AFTER    "ReserveStringBuilderBuffer"
//! INST_NOT      "NewArray (size=2)"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reallocation events occured
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*' --compiler-reserve-string-builder-buffer=false --compiler-inlining=true", entry: "ets_string_builder_reserve_part30-44.ETSGLOBAL::main_events_check"
//! EVENT         "SbBufferRealloc"

//! CHECKER       JIT IR Builder, check StringBuilder internal buffer reallocation events not occured
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*' --compiler-reserve-string-builder-buffer=true --compiler-inlining=true", entry: "ets_string_builder_reserve_part30-44.ETSGLOBAL::main_events_check"
//! EVENT_NOT     "SbBufferRealloc"

function try_catch_reserve30(do_throw: boolean) : string {
    let sb = new StringBuilder()            // applied
    try {
        sb.append(0)
        if (do_throw)
            throw new Error("Error")
    }
    catch (ex) {
        sb.append("1")
        sb.append(ex)
    }
    return sb.toString()
}

function try_finally_reserve31() : string {
    let sb = new StringBuilder()            // applied
    try {
        sb.append(0)
    }
    finally {
        sb.append("1")
    }
    return sb.toString()
}

function out_reserve32() : StringBuilder {
    let sb = new StringBuilder()            // not applied, due to object being stored/returned
    sb.append(0)
    return sb
}

function in_reserve33(sb : StringBuilder) : String {
    sb.append(1)                            // not applied, due to instance allocated outside block
    return sb.toString()
}

let sb_global : StringBuilder = new StringBuilder
function global_reserve34() : String {
    sb_global.append(0)                            // not applied, due to instance allocated outside block
    return sb_global.toString()
}

function chain_reserve35() : String {
    return new StringBuilder().append(0).append(1).toString()   // applied
}

function more_types_reserve36() : String {
    const emptyString = "";
    let nullString: String = new String(null);
    let chArray: FixedArray<char> = [c'!', c'@', c'#', c'$', c'%'];

    let sb: StringBuilder = new StringBuilder(chArray);
    sb.append(' ')
        .append("abcΣΨΩ0123456789")
        .append(' ')
        .append(true)
        .append(' ')
        .append(false)
        .append(' ')
        .append(57 as byte)
        .append(' ')
        .append(127 as byte)
        .append(' ')
        .append(32767 as short)
        .append(' ')
        .append(128934675 as int)
        .append(' ')
        .append(701234987654321 as long)
        .append(' ')
        .append(emptyString)
        .append(nullString);
    return sb.toString();
}

function nested_loop_reserve37() : string {
    let sb_outer = new StringBuilder()      // applied, due loop is countable
    for (let i = 2; i < 8; ++i) {
        sb_outer.append(i)

        let sb_inner = new StringBuilder()  // applied, due loop is countable
        for (let j = 2; j < 11; ++j)
            sb_inner.append(j)

        sb_outer.append(sb_inner.toString())
    }
    return sb_outer.toString()
}

function store_reserve39() : string {
    sb_global = new StringBuilder()            // not applied, due to object being stored
    sb_global.append(0)
    sb_global.append(1)
    sb_global.append(2)
    return sb_global.toString()
}

function chain_reserve40() : String {
    let chars:FixedArray<char> = new FixedArray<char>(1)
    chars[0] = c'0'
    return new StringBuilder(chars).append(1).append(2).toString()   // applied
}

function chain_reserve41(str: string) : String {
    return new StringBuilder(str).append(1).append(2).toString()   // applied
}

function __noinline__foo(sb: StringBuilder)
{
    sb.append(1)
}

function arg_reserve42(): string
{
    let sb = new StringBuilder()                // not applied, due to object being passed to non-inlined function
    sb.append(0)
    __noinline__foo(sb)
    sb.append("2")
    return sb.toString();
}

class StringBuilderHolder
{
    sb : StringBuilder = new StringBuilder()
}
let sb_holder = new StringBuilderHolder
function store_reserve43() : string
{
    let sb = new StringBuilder()                // not applied, due to object being stored
    sb.append(0)

    sb_holder.sb = sb

    sb.append(1)
    return sb.toString();
}

let sb_array = new StringBuilder[1]
function store_reserve44() : string
{
    let sb = new StringBuilder()                // not applied, due to object being stored
    sb.append(0)

    sb_array[0] = sb

    sb.append(1)
    return sb.toString();
}

function main_events_check() : int {
    arktest.assertEQ(try_finally_reserve31(), "01", "Wrong result at try_finally_reserve31");
    arktest.assertEQ(out_reserve32().append(1).toString(), "01", "Wrong result at out_reserve32");
    arktest.assertEQ(in_reserve33(new StringBuilder().append(0)), "01", "Wrong result at in_reserve33");
    arktest.assertEQ(global_reserve34(), "0", "Wrong result at in_reserve34");
    arktest.assertEQ(chain_reserve35(), "01", "Wrong result at chain_reserve35");
    arktest.assertEQ(more_types_reserve36(), "!@#$% abcΣΨΩ0123456789 true false 57 127 32767 128934675 701234987654321 null", "Wrong result at more_types_reserve36");
    arktest.assertEQ(nested_loop_reserve37(), "223456789103234567891042345678910523456789106234567891072345678910", "Wrong result at nested_loop_reserve37");
    arktest.assertEQ(store_reserve39(), "012", "Wrong result at store_reserve39");
    arktest.assertEQ(chain_reserve40(), "012", "Wrong result at chain_reserve40");
    arktest.assertEQ(chain_reserve41("0"), "012", "Wrong result at chain_reserve41");
    arktest.assertEQ(store_reserve43(), "01", "Wrong result at store_reserve43");
    arktest.assertEQ(store_reserve44(), "01", "Wrong result at store_reserve44");

    return 0
}

function main_compile_check() : int {
    arktest.assertTrue(try_catch_reserve30(false).startsWith("0"), "Wrong result at try_catch_reserve30");
    arktest.assertTrue(try_catch_reserve30(true).startsWith("01Error"), "Wrong result at try_catch_reserve30");
    arktest.assertEQ(arg_reserve42(), "012", "Wrong result at arg_reserve42");
    return 0
}

function main() : int {
    main_events_check()
    main_compile_check()
    return 0
}
