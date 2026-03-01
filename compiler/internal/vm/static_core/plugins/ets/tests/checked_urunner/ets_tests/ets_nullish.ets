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

//! CHECKER      AOT IR Builder, check null instructions
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*test_null.*' --compiler-inlining=false"
//!
//! METHOD       "ets_nullish.ETSGLOBAL::test_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /LoadUniqueObject/
//!
//! METHOD       "ets_nullish.ETSGLOBAL::test_null_dce"
//! PASS_AFTER   "Codegen"
//! INST_NOT     /LoadUniqueObject/
//!  
//! METHOD       "ets_nullish.ETSGLOBAL::test_null_cse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadUniqueObject/, 1
//!

//! CHECKER      Force JIT null
//! RUN          force_jit: true, options: "--compiler-regex='.*test_null.*'", entry: "ets_nullish.ETSGLOBAL::test_null"
//! METHOD       "ets_nullish.ETSGLOBAL::test_null"
//! PASS_AFTER   "IrBuilder"
//! INST         /LoadUniqueObject/

//! CHECKER      Force JIT null dce
//! RUN          force_jit: true, options: "--compiler-regex='.*test_null_dce.*'", entry: "ets_nullish.ETSGLOBAL::test_null_dce"
//! METHOD       "ets_nullish.ETSGLOBAL::test_null_dce"
//! PASS_AFTER   "Codegen"
//! INST_NOT     /LoadUniqueObject/

//! CHECKER      Force JIT null cse
//! RUN          force_jit: true, options: "--compiler-regex='.*test_null_cse.*'", entry: "ets_nullish.ETSGLOBAL::test_null_cse"
//! METHOD       "ets_nullish.ETSGLOBAL::test_null_cse"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   /LoadUniqueObject/, 1

let glob: Object | undefined | null = null;

function test_null(): int {
    glob = null;
    return 0;
}

function test_null_dce(): int {
    let v: Object | null = null;
    return 0;
}

function test_null_cse(): int {
    glob = null;
    glob = null;
    return 0;
}
