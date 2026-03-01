/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: If the value of an array index expression is negative,
  or greater than, or equal to the length of the array,
  then an array indexing expression throws RangeError.
---*/

function main() {

  let v: int[][] = [[1, 2], [3, 4], [5, 6]];
  let a = 1;
  let b = 1;

  let s: String = "X" + (a = 2) + ++v[1][1] + (b = 2);
  arktest.assertEQ(a, 2);
  arktest.assertEQ(b, 2);
    
}