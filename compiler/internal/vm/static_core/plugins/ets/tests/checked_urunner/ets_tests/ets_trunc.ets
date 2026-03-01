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

//! CHECKER       Math.trunc JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_trunc.ETSGLOBAL::main"
//! METHOD        "ets_trunc.ETSGLOBAL::main"
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdMathTrunc/, 12
//! ASM_METHOD    "ets_trunc.ETSGLOBAL::main"
//! ASM_INST      "Intrinsic.StdMathTrunc"
//! ASM           x64: /roundsd.*3,/, arm64: "frintz", arm32: "vrintz"

//! CHECKER       Math.trunc AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*main.*'"
//! METHOD        "ets_trunc.ETSGLOBAL::main"
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdMathTrunc/, 12
//! ASM_METHOD    "ets_trunc.ETSGLOBAL::main"
//! ASM_INST      "Intrinsic.StdMathTrunc"
//! ASM           x64: /roundsd.*3,/, arm64: "frintz"
//! RUN           entry: "ets_trunc.ETSGLOBAL::main"

//! CHECKER       Math.trunc LLVM AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_LLVM      options: "--compiler-regex='.*main.*'"
//! RUN           entry: "ets_trunc.ETSGLOBAL::main"

function main(): int {
    if (trunc(Double.POSITIVE_INFINITY) != Double.POSITIVE_INFINITY) return 1;
    if (trunc(Double.NEGATIVE_INFINITY) != Double.NEGATIVE_INFINITY) return 2;
    if (Double.toString(trunc(Double.NaN)) != Double.toString(Double.NaN)) return 3;
    if (trunc(0.) != 0.) return 4;
    if (trunc(1.) != 1.) return 5;
    if (trunc(-1.) != -1.) return 6;
    if (trunc(1.5) != 1.) return 7;
    if (trunc(1.6) != 1.) return 8;
    if (trunc(1.4) != 1.) return 9;
    if (trunc(-1.5) != -1.) return 10;
    if (trunc(-1.6) != -1.) return 11;
    if (trunc(-1.4) != -1.) return 12;
    
    return 0;
}
