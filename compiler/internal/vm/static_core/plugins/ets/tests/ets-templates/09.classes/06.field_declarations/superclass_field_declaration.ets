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
desc: Declare field with name used in superclass.
assert: >-
    When extending a class or implementing interfaces, a field declared
    in a superclass or a superinterface can be overridden by a field
    with the same name, and the same static or non-static modifier status.
    Using the keyword override is not required. The new declaration acts
    as redeclaration.
---*/

class A {
  public x: int = 4;
}

class B extends A {
  public x: int = 5;
}

function main(): void {
    let instance: A = new B();

    arktest.assertEQ(instance.x, 5);
}
