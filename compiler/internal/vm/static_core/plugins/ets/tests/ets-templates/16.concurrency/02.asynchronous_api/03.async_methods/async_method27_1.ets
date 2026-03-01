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
      Using promise.allsettled in asynchronous methods.
---*/

class MethodWithAllSettled {
    async methodWithAllSettled(): Promise<Array<PromiseFulfilledResult<'Resolved!'>|PromiseRejectedResult>>{
        const results = await Promise.allSettled([
            Promise.resolve("Resolved!"),
            Promise.reject(new Error("Rejected!")),
        ]);
        return results;
    }
}

async function startTest() {
    const example = new MethodWithAllSettled();
    const result = await example.methodWithAllSettled();

    arktest.assertTrue(Array.isArray(result), "Result is not instanceof Array");
    arktest.assertEQ(result.length, 2);

    arktest.assertTrue(result[0] instanceof PromiseFulfilledResult, "result[0] is not instanceof PromiseFulfilledResult");
    arktest.assertTrue(result[1] instanceof PromiseRejectedResult, "result[1] is not instanceof PromiseRejectedResult");


    arktest.assertEQ(result[0].status, 'fulfilled');
    arktest.assertEQ(result[1].status, 'rejected');

    arktest.assertEQ((result[1] as PromiseRejectedResult).reason.toString(), 'Error: Rejected!');

    // additional check
    arktest.assertEQ(JSON.stringify(result[0]), '{"status":"fulfilled","value":"Resolved!"}');

}

function main(): void {
    waitForCompletion(startTest);
}