/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER      Check that loads after a non-dominating store are not replaced by a phi
//! RUN          force_jit: true, options: "--compiler-regex='.*lambda.*'", entry: "lse_loop_non_dominating_store.ETSGLOBAL::main"
//! METHOD       "lse_loop_non_dominating_store.__lambda-lambda_invoke_0::invoke1"
//! PASS_BEFORE  "LSE"
//! INST_COUNT   /LoadObject.*std\.core\.IntBox\.value/, 4
//! PASS_AFTER   "LSE"
//! INST_COUNT   /LoadObject.*std\.core\.IntBox\.value/, 2

function test(flag: Boolean): int {
    let i = 0;
    return ((flag: Boolean) => {
        let sum = 0;
        for (i = 0; i < 1; i++) {
            if (flag) {
                i++;
            }
            sum += i;
        }
        return sum;
    })(flag);
}

function main(): void
{
    arktest.assertEQ(test(true), 1);
}
