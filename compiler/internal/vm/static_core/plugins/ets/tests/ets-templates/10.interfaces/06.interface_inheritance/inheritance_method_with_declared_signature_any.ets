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
desc: Inheritance method from superinterface with declared method with same signature in interface.
assert: >-
      An interface I inherits from its direct superinterfaces all abstract and default methods m
      for which all of the following are true - No method declared in I has a signature that is
      a subsignature of the signature of m.
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(): int {
    return 2;
  }
}

class C implements B {
  foo(...p: Any[]): Any {
    return 3
  }
}

function main(): void {
  let instance: C = new C();
  arktest.assertEQ(instance.foo(), 3)
}
