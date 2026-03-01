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
desc: Inheritance open method from superinterface with declared private method with same signature in interface.
assert: >-
    A compile-time error occurs if:
        If an interface I declares a private method m, and
        Signature of m is compatible with the public instance method 𝑚′ in a superinterface of I, and
        𝑚′ is otherwise accessible to code in I.
tags: [negative, compile-only]
---*/

interface A {
  foo(): int {
    return 1;
  }
}

interface B extends A {
  private foo(): int {
    return 2;
  }
}
