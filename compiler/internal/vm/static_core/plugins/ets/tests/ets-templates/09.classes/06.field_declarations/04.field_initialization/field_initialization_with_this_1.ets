/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Non-static field initialization with keyword 'this'.
assert: >-
    The instance field initializer expression cannot use
    this or super directly in any form.
tags: [compile-only, negative]
---*/

class A {

  public x: int = this.bar().y;
  public y: int = 5;

  public bar(): this {
    return this;
  }
}

function main(): void {
    let instance: A = new A();

    arktest.assertEQ(instance.x, 5);
}
