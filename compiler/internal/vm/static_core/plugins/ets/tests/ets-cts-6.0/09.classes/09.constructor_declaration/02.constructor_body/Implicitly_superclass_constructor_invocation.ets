/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: Check that superclass construct implicitly invoked.
assert: >-
    If a constructor body does not begin with an explicit constructor call and the constructor being declared
    is not part of the primordial class Object, then the constructor body implicitly begins
    with a superclass constructor call "super();",
    a call of the constructor of its direct superclass that takes no arguments.
---*/

class A {
  public x: int;

  public constructor() {
    this.x = 5;
  }
}

class B extends A {
  public constructor() {

  }
}

function main(): void {
  let instance: B = new B();
  arktest.assertEQ( instance.x, 5 );
}
