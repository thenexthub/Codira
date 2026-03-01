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

//! CHECKER       AOT, check inteface cache used for 32-bit pointers
//! SKIP_IF       @architecture != "arm64" || is_hybrid_config?
//! RUN_PAOC      options: "--paoc-mode=aot --load-runtimes=ets --compiler-enable-events=true"
//!
//! EVENT         "Codegen,Interface cache used"

//! CHECKER       AOT, check cache not used for 64-bit pointers
//! SKIP_IF       @architecture != "arm64" || !is_hybrid_config?
//! RUN_PAOC      options: "--paoc-mode=aot --load-runtimes=ets --compiler-enable-events=true"
//!
//! EVENT         "Codegen,Interface cache not used"

export interface Interface {

    method(): Boolean;

}

class ActualClass implements Interface {

    method(): Boolean {
        return true
    }

}

function test(iface: Interface): Boolean {
    return iface.method()
}

function main() : void {
    arktest.assertEQ(test(new ActualClass), true)
}
