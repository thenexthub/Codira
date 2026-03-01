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
desc: Method declaration with existed name and with override-equivalent signatures in class body declaration.
assert: >-
    It is a compile-time error for the body of a class declaration to declare as members two methods with the
    same name and with override-equivalent signatures.
tags: [compile-only, negative]
---*/

class A {
  public run(): void {

  }
}

class B extends A {
  public override run(): void {

  }

  public override run(): void {

  }
}
