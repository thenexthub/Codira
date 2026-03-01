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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       Invalid phi instruction AOT (IARN2S/19226)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_concat_loop' --compiler-loop-unroll=false --compiler-inlining=true", entry: "ets_string_concat_loop_part3.ETSGLOBAL::bugfix_concat_loop"

//! CHECKER       Invalid phi instruction JIT (IARN2S/19226)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*bugfix_concat_loop' --compiler-loop-unroll=false", entry: "ets_string_concat_loop_part3.ETSGLOBAL::bugfix_concat_loop"

function bugfix_concat_loop() {
    let str = "";
    let a2 : FixedArray<string> = ["dd"]
    let idx2 : string | boolean;
    for (idx2 of a2) {
        let c2 = idx2
        if (c2 instanceof string) {
            str += c2;
        }
    }

    arktest.assertEQ(str, "dd");
}