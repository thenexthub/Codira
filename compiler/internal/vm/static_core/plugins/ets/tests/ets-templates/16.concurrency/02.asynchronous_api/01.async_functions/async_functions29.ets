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
desc: Test cases for various usage scenarios of asynchronous function.
      Using promise.any in asynchronous functions.
---*/

async function functionWithAny(): Promise<string> {
    const result = await Promise.any([
    new Promise<string>((resolve: (p: string) => void) => {
    setTimeout(() => {
        resolve("Resolved after 100ms");
      }, 100);
    }),
    new Promise<string>((resolve: (p: string) => void) => {
        setTimeout(() => {
            resolve("Resolved after 200ms");
        }, 200);
      }),
    ]);
    return result as string;
}

async function startTest() {
    const result = await functionWithAny();
    arktest.assertEQ(result, "Resolved after 100ms");
}

function main(): void {
  waitForCompletion(startTest);
}
