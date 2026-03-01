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
desc: Test the array index expression to use forEach to iterate over an array.
---*/

function main(): void {
  let numbers: Array<number> = new Array<number>(1, 2, 3, 4, 5);

  numbers.forEach((value, index) => {
    arktest.assertEQ(numbers[index.toInt()], value);
  });
}