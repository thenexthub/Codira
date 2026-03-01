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

//! CHECKER         Test check optimize LenArray and MultiArray two layers
//! RUN             force_jit: true, options: "--compiler-regex='.*main1.*'", entry: "multiarray.ETSGLOBAL::main1"
//! EVENT           /Compilation,multiarray.ETSGLOBAL::main1,.*,COMPILED/
//! METHOD          "multiarray.ETSGLOBAL::main1"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      "LenArray", 2
//! INST_COUNT      "BoundsCheck", 2
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "LenArray", 0
//! PASS_AFTER      "Codegen"
//! INST_COUNT      "BoundsCheck", 0
//! INST_COUNT      "BoundsCheckI", 0

//! CHECKER         Test check optimize LenArray and MultiArray checking work BoundCheck
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*main2.*'"
//! EVENT           /Compilation,multiarray.ETSGLOBAL::main2,.*,COMPILED/
//! METHOD          "multiarray.ETSGLOBAL::main2"
//! PASS_BEFORE     "Peepholes"
//! INST_COUNT      "LenArray", 3
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT      "LenArray", 0
//! RUN             options: "--compiler-enable-jit=false", entry: "multiarray.ETSGLOBAL::main2", result: 1
//! EVENT           /DeoptimizationReason,multiarray.ETSGLOBAL::main2,BOUNDS_CHECK/

//! CHECKER         LenArray and MultiArray with LLVM AOT
//! SKIP_IF         @architecture == "arm32"
//! RUN_LLVM        options: "--compiler-regex='.*main2.*'"
//! RUN             options: "--compiler-enable-jit=false", entry: "multiarray.ETSGLOBAL::main2", result: 1

//! CHECKER         Test check don't optimize LenArray of String in MultiArray
//! RUN             force_jit: true, options: "--compiler-regex='.*main3.*'", entry: "multiarray.ETSGLOBAL::main3"
//! EVENT           /Compilation,multiarray.ETSGLOBAL::main3,.*,COMPILED/
//! METHOD          "multiarray.ETSGLOBAL::main3"
//! PASS_AFTER      "Codegen"
//! INST_COUNT      "LenArray", 1

function main1(): int {
    let M1: FixedArray<FixedArray<int>> = new int[4][4];
    M1[0][0] = 1;
    return 0;
}

function main2(): int {
    let M1: FixedArray<FixedArray<FixedArray<int>>> = new int[2][2][2];
    M1[0][0][3] = 1;
    return 0;
}

function main3(): int {
    let arr: FixedArray<FixedArray<String>> = new String[2][3];
    arr[0][0] = new String();
    return arr[0][0].length  // Should be "0"
}
