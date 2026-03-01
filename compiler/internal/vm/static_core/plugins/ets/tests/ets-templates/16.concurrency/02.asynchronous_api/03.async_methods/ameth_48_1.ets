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
desc: Throw error in chained catch await is forbidden for non-async function
---*/

type EN = Error|null
let s = 'A'
class A {
    async foo(p: string): Promise<string> {
        if (p == 'A') {
            throw new Error('E')
        }
        return 'X'
    }
}

async function startTest(): Promise<void> {
    // Throw error in chained catch
    // await is forbidden for non-async function
    let r = new A().foo(s)
        .then((p: string): string => p + p)
        .catch((p: EN): string => { throw p! })
    try {
        await r
    } catch (e) {
        arktest.assertEQ((e as Error).message, 'E')
        return
    }
    arktest.assertTrue(false)
}

function main(): void {
   waitForCompletion(startTest);
}
