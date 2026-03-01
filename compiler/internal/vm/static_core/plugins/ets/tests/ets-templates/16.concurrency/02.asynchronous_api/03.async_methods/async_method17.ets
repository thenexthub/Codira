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
      Concurrent execution of multiple asynchronous methods.
---*/

class MethodConcurrentExecution {
      async methodReturnString(): Promise<string> {
            return "Hello";
      }
      
      async methodReturnNumber(): Promise<number> {
            return 42.0;
      }
      
      async methodConcurrentExecution(): Promise<Array<String|number>> {
            const result = await Promise.all(new Array<Promise<string|number>>(
                  this.methodReturnString(),
                  this.methodReturnNumber(),
            ));
            
            return result;
      }
}

async function startTest() {
      const example = new MethodConcurrentExecution();
      const result = await example.methodConcurrentExecution();
      arktest.assertTrue(Array.isArray(result));
      arktest.assertEQ(result.length, 2);
      arktest.assertEQ(result[0], "Hello");
      arktest.assertEQ(result[1], 42.0);
}

function main(): void {
      waitForCompletion(startTest);
}
