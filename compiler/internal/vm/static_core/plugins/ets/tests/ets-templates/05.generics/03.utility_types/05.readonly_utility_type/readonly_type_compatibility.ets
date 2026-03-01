/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: Type T is compatible (see Type Compatibility) with Readonly<T>, and allows assignments as a consequence
---*/

interface I {
  i: number;
  j: string;
}

class A implements I {
  i = 0.1;
  j = "";
  constructor(i: number, j: string) {
    this.i = i;
    this.j = j;
  }
  constructor() {}
}

class C {
  i: number = 0.2;
  j: string = "";
  constructor(i: number, j: string) {
    this.i = i;
    this.j = j;
  }
  constructor() {}
}

function main() {
  let z: Readonly<I> = {i: 1.8, j: "some str"}
  arktest.assertTrue(z.i == 1.8)
  arktest.assertTrue(z.j == "some str")
  z = new A(1.9, "new str")
  arktest.assertTrue(z.i == 1.9)
  arktest.assertTrue(z.j == "new str")

  let x: Readonly<A> = new A(1.8, "some str")
  arktest.assertTrue(x.i == 1.8)
  arktest.assertTrue(x.j == "some str")
  x = {i: 1.9, j: "new str"}
  arktest.assertTrue(x.i == 1.9)
  arktest.assertTrue(x.j == "new str")

  let y: Readonly<C> = {i: 1.8, j: "some str"}
  arktest.assertTrue(y.i == 1.8)
  arktest.assertTrue(y.j == "some str")
  y = new C( 1.5, "new str")
  arktest.assertTrue(y.i == 1.5)
  arktest.assertTrue(y.j == "new str")
}
