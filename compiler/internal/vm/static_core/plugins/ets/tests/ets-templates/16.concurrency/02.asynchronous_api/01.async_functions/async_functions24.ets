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
      Asynchronous function returns array.
---*/

async function functionReturnArray(): Promise<Array<number>> {
    return new Array<number>(1, 2, 3);
}

async function startTest() {
  const result = await functionReturnArray();
  const resTemp = new Array<number>(1, 2, 3);
  arktest.assertEQ(result[0], resTemp[0]);
  arktest.assertEQ(result[1], resTemp[1]);
  arktest.assertEQ(result[2], resTemp[2]);
}

function main(): void {
  waitForCompletion(startTest);
}
