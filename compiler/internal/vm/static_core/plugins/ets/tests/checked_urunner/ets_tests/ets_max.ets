/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

//! CHECKER      IR Builder inlining max int
//! RUN          force_jit: true, options: "--compiler-regex='.*test_i32.*'", entry: "ets_max.ETSGLOBAL::test_i32"
//! METHOD       "ets_max.ETSGLOBAL::test_i32"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining max int
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_i32.*'"
//! METHOD        "ets_max.ETSGLOBAL::test_i32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ets_max.ETSGLOBAL::test_i32"

//! CHECKER       IR Builder inlining max long
//! RUN           force_jit: true, options: "--compiler-regex='.*test_i64.*'", entry: "ets_max.ETSGLOBAL::test_i64"
//! METHOD        "ets_max.ETSGLOBAL::test_i64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining max long
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_i64.*'"
//! METHOD        "ets_max.ETSGLOBAL::test_i64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 6
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ets_max.ETSGLOBAL::test_i64"

//! CHECKER       IR Builder inlining max float
//! RUN           force_jit: true, options: "--compiler-regex='.*test_f32.*'", entry: "ets_max.ETSGLOBAL::test_f32"
//! METHOD        "ets_max.ETSGLOBAL::test_f32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 10
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining max float
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_f32.*'"
//! METHOD        "ets_max.ETSGLOBAL::test_f32"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 10
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ets_max.ETSGLOBAL::test_f32"

//! CHECKER       IR Builder inlining max double
//! RUN           force_jit: true, options: "--compiler-regex='.*test_f64.*'", entry: "ets_max.ETSGLOBAL::test_f64"
//! METHOD        "ets_max.ETSGLOBAL::test_f64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 20
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining max double
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*test_f64.*'"
//! METHOD        "ets_max.ETSGLOBAL::test_f64"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Max/, 20
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ets_max.ETSGLOBAL::test_f64"


function test_i32() : int {
    if (max(Int.MAX_VALUE as int, 0 as int) != Int.MAX_VALUE as int) return 1;
    if (max(Int.MIN_VALUE as int, 0 as int) != 0 as int) return 2;
    if (max(0 as int, -2 as int) != 0 as int) return 3;
    if (max(0 as int, 3 as int) != 3 as int) return 4;
    if (max(-2 as int, -4 as int) != -2 as int) return 5;
    if (max(1 as int, 4 as int) != 4 as int) return 6;

    return 0;
}

function test_i64() : int {
    if (max(Long.MAX_VALUE, 0) != Long.MAX_VALUE) return 11;
    if (max(Long.MIN_VALUE, 0) != 0) return 12;
    if (max(0 as int, -2) != 0) return 13;
    if (max(0 as int, 3) != 3) return 14;
    if (max(-2 as int, -4) != -2) return 15;
    if (max(1 as int, 4) != 4) return 16;

    return 0;
}

function test_f32() : int {
    if (max(Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY) != Float.POSITIVE_INFINITY) return 21;
    if (max(Float.MAX_VALUE, Float.NEGATIVE_INFINITY) != Float.MAX_VALUE) return 22;
    if (max((0.).toFloat(), Float.NEGATIVE_INFINITY) != 0 as float) return 23;
    if (max((0).toFloat(), Float.MIN_VALUE) != Float.MIN_VALUE) return 24;
    if (max(Float.MAX_VALUE, Float.MIN_VALUE) != Float.MAX_VALUE) return 25;
    if (max((0).toFloat(), (2).toFloat()) != (2).toFloat()) return 26;
    if (max((0).toFloat(), (-4).toFloat()) != (0).toFloat()) return 27;
    if (max((1.e-10).toFloat(), (2.e+12).toFloat()) != (2.e+12).toFloat()) return 28;
    if (!Float.isNaN(max(Float.NaN, Float.NaN))) return 29;
    if (!Float.isNaN(max(Float.MAX_VALUE, Float.NaN))) return 30;

    return 0;
}

function test_f64() : int {
    if (max(Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY) != Double.POSITIVE_INFINITY) return 31;
    if (max(Double.MAX_VALUE, Double.NEGATIVE_INFINITY) != Double.MAX_VALUE) return 32;
    if (max(0, Double.NEGATIVE_INFINITY) != 0) return 33;
    if (max(0, Double.MIN_VALUE) != Double.MIN_VALUE) return 34;
    if (max(Double.MAX_VALUE, Double.MIN_VALUE) != Double.MAX_VALUE) return 35;
    if (max(0 as double, 2) != 2) return 36;
    if (max(0 as double, -4) != 0) return 37;
    if (max(1.e-100, 2.e+200) != 2.e+200) return 38;
    if (!Double.isNaN(max(Double.NaN, Double.NaN))) return 39;
    if (!Double.isNaN(max(Double.MAX_VALUE, Double.NaN))) return 40;

    if (Math.max(Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY) != Double.POSITIVE_INFINITY) return 41;
    if (Math.max(Double.MAX_VALUE, Double.NEGATIVE_INFINITY) != Double.MAX_VALUE) return 42;
    if (Math.max(0, Double.NEGATIVE_INFINITY) != 0) return 43;
    if (Math.max(0, Double.MIN_VALUE) != Double.MIN_VALUE) return 44;
    if (Math.max(Double.MAX_VALUE, Double.MIN_VALUE) != Double.MAX_VALUE) return 45;
    if (Math.max(0 as double, 2) != 2) return 46;
    if (Math.max(0 as double, -4) != 0) return 47;
    if (Math.max(1.e-100, 2.e+200) != 2.e+200) return 48;
    if (!Double.isNaN(Math.max(Double.NaN, Double.NaN))) return 49;
    if (!Double.isNaN(Math.max(Double.MAX_VALUE, Double.NaN))) return 50;

    return 0;
}
