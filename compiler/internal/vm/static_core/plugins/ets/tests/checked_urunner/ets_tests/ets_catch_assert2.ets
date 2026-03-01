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

//! CHECKER      Try Catch Resolving JIT Optimize with false assert to check correctness of LSE optimization
//! SKIP_IF       @architecture == "arm32"
//! RUN          options: " --compiler-enable-jit=true --compiler-check-final=true --gc-type=g1-gc --heap-verifier=fail_on_verification:pre:into:before_g1_concurrent:post --compiler-hotness-threshold=0 --no-async-jit=true --full-gc-bombing-frequency=0", entry: "ets_catch_assert2.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_catch_assert2.ETSGLOBAL::main,.*,COMPILED/

let a = 12;
let b = 11;
let c = 10;
let d = 9;
let res = "";

function foo(): string {
    res += "foo called";
    return "Assert message";
}

function main(): int {
    res = "";
    try {
        if (a >= b) {
            if (b >= c) {
                if (c >= d) {
                    foo();
                    arktest.assertTrue(false);
                }
            }
        }
    } catch(e) {
        // expected
    }
    return res == "" ? 1 : 0;
}
