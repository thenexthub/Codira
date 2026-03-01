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
desc: Override inheritance methods.
assert: The interface inherits, from the interfaces it extends, all members of those interfaces, except for methods that it overrides.
---*/

interface A {
  m(): int { return 5; }
}

interface B extends A {
  m(): int { return 10; }
}

class C implements B {
  public bar(): int {
    return this.m();
  }
}

function main(): void {
  let x: C = new C();
  arktest.assertEQ(x.bar(), 10)
}
