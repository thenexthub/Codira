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
      Using try-catch to catch errors in asynchronous methods.
---*/

class MethodWithErrorHandling {
    async methodThrowError(): Promise<void> {
        throw new Error("Something went wrong!");
    }

    async methodWithErrorHandling(): Promise<string> {
        try {
            await this.methodThrowError();
            return "Success";
        } catch (error) {
            return `Caught error: ${error.message as string}`;
        }
    }
}

async function startTest() {
    const example = new MethodWithErrorHandling();
    const result = await example.methodWithErrorHandling();
    arktest.assertEQ(result, "Caught error: Something went wrong!")
}

function main(): void {
    waitForCompletion(startTest);
}