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
desc: Declaring subinterface
assert: If an extends clause is provided, then the interface being declared extends each of the other named interfaces and therefore inherits the member types, instance methods, and constants of each of the other named interfaces.
tags: [compile-only]
---*/

interface A {
  foo(): int {
    return 5;
  }
}

interface B {
  bar(): int {
    return 5;
  }
}

interface C extends A, B {}

class D implements C {
    public sum(): void {
        let r: int = this.foo() + this.bar();
    }
}
