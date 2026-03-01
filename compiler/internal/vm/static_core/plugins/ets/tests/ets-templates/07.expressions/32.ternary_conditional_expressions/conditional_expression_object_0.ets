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
desc: Second operand is value type and third operand are reference type
---*/

final class A {}
final class B {}

type BRF = (input: boolean) => boolean

function main(): void {

  const bf: BRF = (input: boolean) => { return input }

  let a: int = 5;

  let x = bf(true) ? a : new B;
  arktest.assertTrue(x instanceof Int|B)
}
