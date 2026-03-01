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

//! CHECKER       Div/Mod by const optimization, JIT
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*(Div|Mod).*'", entry: "fast_divisor.ETSGLOBAL::main"
//! EVENT_NOT     /Compilation,fast_divisor.ETSGLOBAL::main,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntModByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongModByMin,.*,COMPILED/
//! METHOD        "fast_divisor.ETSGLOBAL::IntDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::IntDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::IntDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::IntModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::IntModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::IntModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     "AndI"
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::LongDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::LongDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::LongDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::LongModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::LongModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::LongModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     /And|AndI/
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"

//! CHECKER       Div/Mod by const optimization, AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*(Div|Mod).*'"
//! EVENT_NOT     /Compilation,fast_divisor.ETSGLOBAL::main,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongDivBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongModBy7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongDivByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongModByM7,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::IntModByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongDivByMin,.*,COMPILED/
//! EVENT         /Compilation,fast_divisor.ETSGLOBAL::LongModByMin,.*,COMPILED/
//! METHOD        "fast_divisor.ETSGLOBAL::IntDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::IntDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::IntDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::IntModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::IntModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smull"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::IntModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     "AndI"
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::IntModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::LongDivBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongDivBy7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::LongDivByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "DivI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongDivByM7"
//! ASM_INST      "DivI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! METHOD        "fast_divisor.ETSGLOBAL::LongDivByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Div",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Div",0
//! INST          "AShrI"
//! INST_NEXT     "ShrI"
//! INST_NEXT     "Add"
//! INST_NEXT     "AShrI"
//! INST_NEXT     "Neg"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongDivByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! METHOD        "fast_divisor.ETSGLOBAL::LongModBy7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongModBy7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::LongModByM7"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "ModI",1
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongModByM7"
//! ASM_INST      "ModI"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! ASM           x64: "imul", arm64: "smulh"
//! ASM           x64: "sar", arm64: "asr"
//! ASM           x64: "sub", arm64: "msub"
//! METHOD        "fast_divisor.ETSGLOBAL::LongModByMin"
//! PASS_BEFORE   "Lowering"
//! INST_COUNT    "Mod",1
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT    "Mod",0
//! INST          "Add"
//! INST_NEXT     "SelectImm"
//! INST_NEXT     /And|AndI/
//! INST_NEXT     "Sub"
//! ASM_METHOD    "fast_divisor.ETSGLOBAL::LongModByMin"
//! ASM_NOT       x64: "idiv", arm64: "sdiv"
//! RUN           entry: "fast_divisor.ETSGLOBAL::main"

// Int Div/Mod by 7
function IntDivBy7(num: int): int {
    return num / 7;
}

function IntModBy7(num: int): int {
    return num % 7;
}

// Long Div/Mod by 7
function LongDivBy7(num: long): long {
    return num / 7;
}

function LongModBy7(num: long): long {
    return num % 7;
}

// Int Div/Mod by (-7)
function IntDivByM7(num: int): int {
    return num / (-7);
}

function IntModByM7(num: int): int {
    return num % (-7);
}

// Long Div/Mod by (-7)
function LongDivByM7(num: long): long {
    return num / (-7);
}

function LongModByM7(num: long): long {
    return num % (-7);
}

// Int Div/Mod by Int.MIN_VALUE
function IntDivByMin(num: int): int {
    arktest.assertEQ(Int.MIN_VALUE, -2147483648)
    return num / (-2147483648 as int);
}

function IntModByMin(num: int): int {
    arktest.assertEQ(Int.MIN_VALUE, -2147483648)
    return num % (-2147483648 as int);
}

// Long Div/Mod by Long.MIN_VALUE
function LongDivByMin(num: long): long {
    arktest.assertEQ(Long.MIN_VALUE, -9223372036854775808)
    return num / (-9223372036854775808 as long);
}

function LongModByMin(num: long): long {
    arktest.assertEQ(Long.MIN_VALUE, -9223372036854775808)
    return num % (-9223372036854775808 as long);
}

function main(): int {
    arktest.assertEQ(IntDivBy7(1405), 200)
    arktest.assertEQ(IntModBy7(1405), 5)
    arktest.assertEQ(LongDivBy7(1405), 200)
    arktest.assertEQ(LongModBy7(1405), 5)

    arktest.assertEQ(IntDivByM7(1405), -200)
    arktest.assertEQ(IntModByM7(1405), 5)
    arktest.assertEQ(LongDivByM7(1405), -200)
    arktest.assertEQ(LongModByM7(1405), 5)

    arktest.assertEQ(IntDivByMin(1405), 0)
    arktest.assertEQ(IntModByMin(1405), 1405)
    arktest.assertEQ(LongDivByMin(1405), 0)
    arktest.assertEQ(LongModByMin(1405), 1405)

    arktest.assertEQ(IntDivByMin(Int.MIN_VALUE), 1)
    arktest.assertEQ(IntModByMin(Int.MIN_VALUE), 0)
    arktest.assertEQ(LongDivByMin(Long.MIN_VALUE), 1)
    arktest.assertEQ(LongModByMin(Long.MIN_VALUE), 0)

    return 0;
}
