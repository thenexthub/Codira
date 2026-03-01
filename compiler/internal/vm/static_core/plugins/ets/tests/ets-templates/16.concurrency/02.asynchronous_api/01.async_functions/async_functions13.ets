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
      SetTimeout in asynchronous functions simulates delay.
---*/

async function asyncLoop(): Promise<void> {
  for (let i = 1; i <= 3; i++) {
    await new Promise<number>((resolve: (p: number) => void) => {
      setTimeout(() => resolve(10), 500);
    });
  }
}

async function startTest() {
  const start = Date.now();
  await asyncLoop()
  const end = Date.now();
  arktest.assertEQ((end - start) >= 1500, true);
}

function main(): void {
  waitForCompletion(startTest);
}
