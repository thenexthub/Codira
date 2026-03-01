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
desc: Properties of Partial<T> type must be properly initialized with default or predefined values.
---*/

interface I {
  i: number;
}

class A implements I {
  j: string = "";
  i: number = 1;
}

class B extends A {
  k: boolean = false;
}

function main() {
  let f: Partial<B> = { }
  arktest.assertTrue( !f.k )
  arktest.assertTrue( f.i == undefined )
  arktest.assertTrue( f.j == undefined )

  f = { k: false }
  arktest.assertTrue( !f.k )
  arktest.assertTrue( f.i == undefined )
  arktest.assertTrue( f.j == undefined )
  f.i = 2;
  f.j = "new str"
  arktest.assertTrue( f.i == 2 )
  arktest.assertTrue( f.j == "new str" )

  f = { j: "str" }
  arktest.assertTrue( !f.k )
  arktest.assertTrue( f.i == undefined )
  arktest.assertTrue( f.j == "str" )

  f = { i: 4 }
  arktest.assertTrue( !f.k )
  arktest.assertTrue( f.i == 4 )
  arktest.assertTrue( f.j == undefined )

  f = { i: 5, j: "some str" }
  arktest.assertTrue( !f.k )
  arktest.assertTrue( f.i == 5 )
  arktest.assertTrue( f.j == "some str" )

  f = { k: true , i: 5, j: "some str" }
  arktest.assertTrue( f.k == true )
  arktest.assertTrue( f.i == 5 )
  arktest.assertTrue( f.j == "some str" )

}
