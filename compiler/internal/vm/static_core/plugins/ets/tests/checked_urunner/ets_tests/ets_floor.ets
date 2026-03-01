/*
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

//! CHECKER       Math.floor JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_floor.ETSGLOBAL::main"
//! METHOD        "ets_floor.ETSGLOBAL::main"
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdMathFloor/, 12
//! ASM_METHOD    "ets_floor.ETSGLOBAL::main"
//! ASM_INST      "Intrinsic.StdMathFloor"
//! ASM           x64: /roundsd.*1,/, arm64: "frintm", arm32: "vrintm"

//! CHECKER       Math.floor AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*main.*'"
//! METHOD        "ets_floor.ETSGLOBAL::main"
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdMathFloor/, 12
//! ASM_METHOD    "ets_floor.ETSGLOBAL::main"
//! ASM_INST      "Intrinsic.StdMathFloor"
//! ASM           x64: /roundsd.*1,/, arm64: "frintm"
//! RUN           entry: "ets_floor.ETSGLOBAL::main"

//! CHECKER       Math.floor LLVM AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_LLVM      options: "--compiler-regex='.*main.*'"
//! RUN           entry: "ets_floor.ETSGLOBAL::main"

function main(): int {
    if (floor(Double.POSITIVE_INFINITY) != Double.POSITIVE_INFINITY) return 1;
    if (floor(Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) return 2;
    if (Double.toString(floor(Double.NaN)) != Double.toString(Double.NaN)) return 3;
    if (floor(0.) != 0.) return 4;
    if (floor(1.) != 1.) return 5;
    if (floor(-1.) != -1.) return 6;
    if (floor(1.5) != 1.) return 7;
    if (floor(1.6) != 1.) return 8;
    if (floor(1.4) != 1.) return 9;
    if (floor(-1.5) != -2.) return 10;
    if (floor(-1.6) != -2.) return 11;
    if (floor(-1.4) != -2.) return 12;
    
    return 0;
}
