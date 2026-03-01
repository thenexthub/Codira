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
desc: ArithmeticError is thrown by an integer division or integer
  remainder operator if the right-hand operand expression has the zero value.
params: 1/1
---*/

function main() {

  let a = 1;
  let b = 1;
  let i = 1;
  let j = 1;

  let v: String = "X" + (a = 2) + i % j-- + (b = 2);

  arktest.assertEQ(a, 2)
  arktest.assertEQ(b, 2)
}
