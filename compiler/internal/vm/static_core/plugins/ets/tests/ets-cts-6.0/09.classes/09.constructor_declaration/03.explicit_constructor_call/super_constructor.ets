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
desc: >-
    Superclass constructor calls (used to call a direct superclass constructor),
    called Unqualified superclass constructor calls that start with the keyword super
---*/

class A {
  public x: int;

  public constructor(a: int) {
    this.x = a;
  }
}

class B extends A {
  public constructor() {
    super(5);
  }
}

function main(): void {
  let i1: B = new B();

  arktest.assertEQ(i1.x, 5);
}
