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
desc: Default class constructor invoke default constructor of superclass.
assert: >-
    If the class being declared is the primordial class Object, then the default constructor has an empty body.
    Otherwise, the default constructor simply invokes the superclass constructor with no arguments.
---*/

class A {
  public x: int;
  public b1: B | undefined;
  public b2: B | null = null;
}

class B extends A {

}

function main(): void {
  let instance: B = new B();

  arktest.assertEQ(instance.x, 0);
  arktest.assertEQ(instance.b1, undefined);
  arktest.assertEQ(instance.b2, null);
}
