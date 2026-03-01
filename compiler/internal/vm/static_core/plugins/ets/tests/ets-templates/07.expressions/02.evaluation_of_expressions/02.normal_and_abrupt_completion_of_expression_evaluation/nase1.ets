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
  let sz: int = -1;
  let a: int = 1;
  let b: int = 1;
  let expectedException = (e: Error): boolean => { return (e instanceof NegativeArraySizeError) }
  arktest.expectThrow(() => { let v: String = "" + (a = 2) + new byte[sz].length + (b = 2); }, expectedException)
  arktest.assertEQ(a != 2 || b != 1, false)
}
