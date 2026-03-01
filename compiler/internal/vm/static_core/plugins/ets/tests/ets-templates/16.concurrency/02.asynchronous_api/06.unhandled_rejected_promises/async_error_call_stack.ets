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
desc: Async function throws Error
assert: Exception rethrown to main; message is true and typeof err.stack === "string".
---*/

async function startTest(): Promise<void> {
    throw new Error("test-1: thrown from async startTest");
}

function main(): void {
    try {
        waitForCompletion(startTest);
        
        // If we got here, the test failed.
        throw new Error("waitForCompletion did not propagate rejection");
    } catch (e) {
        let err = e as Error;
        arktest.assertEQ(err.message, "test-1: thrown from async startTest");
        arktest.assertEQ(typeof err.stack, "string");
        
        const stackLines = (err.stack as string).split('\n');
        const stackPattern = new RegExp("\\s*at .*(\\d+):(\\d+)");
        for (const line of stackLines) {
            if (line.trim() === '') {
                continue;
            }

            arktest.assertEQ(stackPattern.test(line), true);
        }
    }
}
