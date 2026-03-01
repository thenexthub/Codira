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
desc: Inheritance abstract method of superclass and default methods from superinterface.
assert: >-
    A class C inherits from its direct superclass and direct superinterfaces all abstract
    and default methods m for which all of the following are true.
    m is a member of the direct superclass or a direct superinterface, D, of C. m is public,
    protected, or declared with package access in the same package as C.
    No method declared in C has a signature that is a subsignature of the signature of m.
    No concrete method inherited by C from its direct superclass has a signature that is a subsignature
    of the signature of m. There exists no method m' that is a member of the direct superclass
    or a direct superinterface, D', of C (m distinct from m', D distinct from D'), such that m' overrides
    from D' the declaration of the method m.
---*/

abstract class A {
  abstract a(): int;
}

interface D {
  d(): int {
    return 1;
  }
}

class B extends A implements D {
  override a(): int {
    return 1;
  }

  fee(): int {
    return this.a() + this.d();
  }
}

function main(): void {
  let instance: B = new B();

  arktest.assertEQ( instance.fee(), 2 );
}
