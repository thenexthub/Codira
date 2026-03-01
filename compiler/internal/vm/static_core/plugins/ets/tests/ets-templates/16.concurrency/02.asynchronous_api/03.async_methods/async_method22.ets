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

/*---
desc: Test cases for various usage scenarios of asynchronous method.
      Using conditional statements in asynchronous methods.
---*/

class MethodWithCondition {
    async methodWithCondition(flag: boolean): Promise<string> {
        if (flag) {
            return "Flag is true!";
        } else {
            return "Flag is false!";
        }
    }
}

async function startTest() {
    const example = new MethodWithCondition();
    const result = await example.methodWithCondition(true);
    const result1 = await example.methodWithCondition(false);
    arktest.assertEQ(result, "Flag is true!");
    arktest.assertEQ(result1, "Flag is false!");
}

function main(): void {
    waitForCompletion(startTest);
}