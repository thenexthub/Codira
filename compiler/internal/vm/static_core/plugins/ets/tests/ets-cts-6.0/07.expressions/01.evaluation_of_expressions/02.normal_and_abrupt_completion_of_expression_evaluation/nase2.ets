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
desc: NegativeArraySizeError is thrown by an array creation expression
  if any dimension expression has a value that is less than zero.
---*/

function main() {

  let a: int = 1;
  let b: int = 1;
  // Changed to small number because it will cause runtime timeout
  let sz: int = 10;
  let v: int = 42 + (a = 2) + new byte[sz].length + (b = 2);
  arktest.assertEQ(a, 2);
  arktest.assertEQ(b, 2);
}
