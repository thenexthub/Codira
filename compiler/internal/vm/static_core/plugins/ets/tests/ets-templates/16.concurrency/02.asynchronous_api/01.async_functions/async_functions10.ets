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
      The await chain in asynchronous functions.
---*/

async function step1(): Promise<string> {
  return "Step 1 completed";
}

async function step2(input: string): Promise<string> {
  return `${input}, Step 2 completed`;
}

async function step3(input: string): Promise<string> {
  return `${input}, Step 3 completed`;
}

async function startTest() {
  const result1 = await step1();
  const result2 = await step2(result1);
  const result3 = await step3(result2);
  arktest.assertEQ(result3, "Step 1 completed, Step 2 completed, Step 3 completed");
}

function main(): void {
  waitForCompletion(startTest);
}
