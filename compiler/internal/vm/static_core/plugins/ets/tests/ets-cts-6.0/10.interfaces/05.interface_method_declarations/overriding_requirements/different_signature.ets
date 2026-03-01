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
desc: Overload public default method from superinterface with different signature.
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  foo(c: int): int {
    return 2;
  }
}

class C implements B {}

function main(): void {
  let c: C = new C()
  arktest.assertEQ(c.foo(), 1)
  arktest.assertEQ(c.foo(1), 2)
}
