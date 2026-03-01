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
desc: Test cases for various usage scenarios of asynchronous functions.
      An asynchronous function returns a union type.
---*/

async function testAsync(a: number): Promise<string | number> {
    let b: number = 20;
    return a > 5 ? "text" : b;
}

async function startTest() {
    let result1: string | number = await testAsync(10);
    let result2: string | number = await testAsync(1);
    arktest.assertEQ(result1, "text");
    arktest.assertEQ(result2, 20);
}

function main(): void {
    waitForCompletion(startTest);
}
