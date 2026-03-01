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
desc: Non-static method call.
assert: >-
    An instance method is always invoked with respect to an object, which becomes the current object
    to which the keyword this and super refer during execution of the method body.
---*/

class A {
  public foo(): int {
    return 5;
  }
}

class B extends A {
  public x: int;

  public static run(): int {
    return 5;
  }

  public bar(): int {
    this.x = 5;
    return super.foo() + this.x;
  }
}

function main(): void {
  let instance: B = new B();

  arktest.assertTrue( instance.bar() == 10 );
}
