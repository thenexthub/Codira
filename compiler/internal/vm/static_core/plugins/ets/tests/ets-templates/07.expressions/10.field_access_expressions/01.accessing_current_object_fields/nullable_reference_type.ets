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
desc: Access to field by nullable reference type
assert: An expression must be a non-nullable reference type T to be used for Field Access. Otherwise, a compile-time error occurs.
tags: [negative, compile-only]
---*/

class A {
  public x: int;
}

function main(): void {
    let instance?: A = new A();

    arktest.assertTrue( instance.x == 1 )
}
