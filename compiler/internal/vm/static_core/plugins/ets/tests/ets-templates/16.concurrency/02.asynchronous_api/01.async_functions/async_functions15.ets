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
      Promise.race in asynchronous functions.
---*/

async function raceExample(): Promise<string> {
    const promise1 = new Promise<string>((resolve: (p: string) => void) => {
        setTimeout(() => resolve("Promise 1 resolved"), 500);
    });
    const promise2 = new Promise<string>((resolve: (p: string) => void) => {
        setTimeout(() => resolve("Promise 2 resolved"), 1000);
    });
    return await Promise.race([promise1, promise2]);
}

async function startTest() {
    const result = await raceExample();
    arktest.assertEQ(result === "Promise 1 resolved", true);
}

function main(): void {
  waitForCompletion(startTest);
}
