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
desc: |-
    if types A and B do not overlap (and therefore an expression always evaluated to false at compile time), then:
    - if each of A and B is either a predefined type or a union of predefined types, a compile-time-error is issued.
    - in all other cases, a compile-time warning is issued.
---*/

class A {}
class B {}

function main() {
  let a: A = new A();
  let b: B = new B();
  arktest.assertTrue(a != b)
  arktest.assertTrue(a !== b)
  arktest.assertFalse(a == b)
  arktest.assertFalse(a === b)
}
