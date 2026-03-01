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

//! CHECKER       Math.mod JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_mod.ETSGLOBAL::main"
//! METHOD        "ets_mod.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic/

//! CHECKER       Math.mod AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*main.*'"
//! METHOD        "ets_mod.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic/
//! RUN           entry: "ets_mod.ETSGLOBAL::main"

function main(): int {
    if (mod(2.0, 1.0) != 0.) return 1;
    
    return 0;
}