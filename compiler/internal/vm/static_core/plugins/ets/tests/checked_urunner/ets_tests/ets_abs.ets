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

//! CHECKER       IR Builder inlining abs
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_abs.ETSGLOBAL::main"
//! METHOD        "ets_abs.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Abs/, 9
//! INST_NEXT_NOT /Intrinsic/

//! CHECKER       AOT IR Builder inlining abs
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*main.*'"
//! METHOD        "ets_abs.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_COUNT    /Abs/, 9
//! INST_NEXT_NOT /Intrinsic/
//! RUN           entry: "ets_abs.ETSGLOBAL::main"

function main(): int {
    if (abs(Double.POSITIVE_INFINITY) != Double.POSITIVE_INFINITY) return 1;
    if (abs(Double.NEGATIVE_INFINITY) != Double.POSITIVE_INFINITY) return 2;
    if (abs(Double.MAX_VALUE) != Double.MAX_VALUE) return 3;
    if (abs(-Double.MAX_VALUE) != Double.MAX_VALUE) return 4;
    if (abs(Double.MIN_VALUE) != Double.MIN_VALUE) return 5;
    if (abs(-Double.MIN_VALUE) != Double.MIN_VALUE) return 6;
    if (abs(1.) != 1.) return 7;
    if (abs(-1.) != 1.) return 8;
    if (abs(0.) != 0.) return 9;

    return 0;
}
