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
    async methodWithAllSettled(): Promise<PromiseSettledResult<string>[]> {
        const promises: FixedArray<string | PromiseLike<string>> = [
            Promise.resolve("Resolved!"), 
            Promise.reject(new Error("Rejected!"))
        ]
        return Promise.allSettled(promises)
    }
}

async function startTest() {
    const example = new MethodWithAllSettled();
    const result = await example.methodWithAllSettled();

    arktest.assertEQ(result.length, 2)

    const fulfilledResult = result[0]
    arktest.assertTrue(fulfilledResult instanceof PromiseFulfilledResult)
    if (fulfilledResult instanceof PromiseFulfilledResult) {
        arktest.assertEQ(fulfilledResult.value, "Resolved!")
    }

    const rejectedResult = result[1]
    arktest.assertTrue(rejectedResult instanceof PromiseRejectedResult)
    if (rejectedResult instanceof PromiseRejectedResult) {
        const rejectionError = rejectedResult.reason
        arktest.assertEQ(rejectionError.name, "Error")
        arktest.assertEQ(rejectionError.message, "Rejected!")
    }
}

function main(): void {
    waitForCompletion(startTest);
}
