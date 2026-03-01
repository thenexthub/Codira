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

//! CHECKER      Check overflow check in BoundsAnalysis
//! RUN          force_jit: true, options: "--compiler-regex='.*ETSGLOBAL::crash' --compiler-inlining=false", entry: "bounds_analysis_overflow.ETSGLOBAL::main"
//! EVENT        /Exception,bounds_analysis_overflow.ETSGLOBAL::crash,.*,ARITHMETIC/
//! METHOD       "bounds_analysis_overflow.ETSGLOBAL::crash"
//! PASS_AFTER   "Codegen"
//! INST         "ZeroCheck"

function crash(x: long): long {
    if (x >= 0) {
        let y = x + x + 2
        return x / y
    }
    return -1
}

function main(): int {
    let x = (1 as long) << 62
    let long_max = x - 1 + x
    try {
        let res = crash(long_max);
    } catch (e) {
      if (e instanceof ArithmeticError) {
        return 0;
      }
    }
    return 1;
}