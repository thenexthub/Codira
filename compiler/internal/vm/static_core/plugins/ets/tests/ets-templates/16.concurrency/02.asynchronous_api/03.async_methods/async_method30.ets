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
      using async functions as parameters in asynchronous methods.
---*/

class MethodWithAsyncFunctionAsParam {
    async methodWithAsyncFunctionAsParam(callback: () => Promise<string>): Promise<string> {
        return callback();
    }
}

async function startTest() {
    const example = new MethodWithAsyncFunctionAsParam();
    const result = await example.methodWithAsyncFunctionAsParam(async () => {
        return "Resolved from async function!";
    });
    arktest.assertEQ(result, "Resolved from async function!");
}

function main(): void {
    waitForCompletion(startTest);
}