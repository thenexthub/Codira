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
      Execute multiple asynchronous functions concurrently.
---*/

async function testAsync1(): Promise<string> {
    return "testAsync1";
}

async function testAsync2(): Promise<string> {
    return "testAsync2";
}

async function startTest() {
    const res = await Promise.all([testAsync1(), testAsync2()]);
    arktest.assertEQ(res[0] === "testAsync1", true);
    arktest.assertEQ(res[1] === "testAsync2", true);
}

function main(): void {
    waitForCompletion(startTest);
}
