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

//! CHECKER       Runtime equals JIT
//! RUN           force_jit: true, options: "--compiler-regex='.*main.*'", entry: "ets_runtime_equals.ETSGLOBAL::main"
//! METHOD        "ets_runtime_equals.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /CallVirtual/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic/
//! INST_NOT      /CallVirtual/

//! CHECKER       Runtime equals AOT
//! SKIP_IF       @architecture == "arm32"
//! RUN_AOT       options: "--compiler-regex='.*main.*'"
//! METHOD        "ets_runtime_equals.ETSGLOBAL::main"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /CallVirtual/
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /Intrinsic/
//! INST_NOT      /CallVirtual/
//! RUN           entry: "ets_runtime_equals.ETSGLOBAL::main"

function main(): int {
    let obj1: Object = new Object();
    let obj2: Object = new Object();
    if (Runtime.isSameReference(obj1, null)) return 1;
    if (Runtime.isSameReference(obj1, obj2)) return 2;
    if (!Runtime.isSameReference(null, null)) return 3;
    if (!Runtime.isSameReference(obj1, obj1)) return 4;
    if (obj1 == null) return 5;
    if (obj1 == obj2) return 6;

    return 0;
}
