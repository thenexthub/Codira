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

//! CHECKER      Check for method->isStatic assert
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_promise_launch.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_promise_launch.ETSGLOBAL::main,.*,COMPILED/

//! CHECKER       AOT Check for method->isStatic assert
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*main.*'"

import { launch } from "std/concurrency"

function main(): int {
    for (let i = 0; i < 100; i++) {
        let r = launch<number, (p:number) => number>(((p: number): number => p + p), 42.0)
        arktest.assertEQ((r.Await()), 84.0);
    }
    return 0;
}
