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
desc: null safety -> Null-Coalescing Operator.
assert: The type of null-coalescing expression is the least upper bound of the non-nullable variant of the type of the left-hand-side expression and the type of the right-hand-side expression.
---*/

class A {}

function foo(p: A): int {
  return 2;
}

function main(): void {
  let a : A = new A();
  let b : A = new A();
  let c = a ?? b;
  arktest.assertEQ(foo(c), 2);
}
