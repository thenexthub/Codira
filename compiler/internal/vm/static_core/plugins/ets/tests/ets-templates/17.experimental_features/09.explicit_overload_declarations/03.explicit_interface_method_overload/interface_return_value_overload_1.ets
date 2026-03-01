/*---
Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
desc: >-
    A method is defined in a subinterface with the same name as the method in the superinterface.
    If signatures are override-compatible (see Override-Compatible Signatures), then overriding is used.
    Otherwise, overloading is used.
---*/

let res: number = 0;

interface I {
  foo1(i: byte): void {
    res = 1;
  }
}

interface J extends I {
  foo2(j: int): number {
      res = 2;
      return 2.1;
  }
}

class A implements J {
   overload foo { foo1, foo2 }
}

function main(): void {
  let a: A = new A();
  a.foo(1)
  arktest.assertTrue(res == 1);
  a.foo(1 as byte);
  arktest.assertTrue(res == 1);
}
