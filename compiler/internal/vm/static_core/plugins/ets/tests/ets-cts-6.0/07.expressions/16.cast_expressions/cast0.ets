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
desc: >-
    The cast operator converts the value V of one type (as denoted
    by the expression) at runtime to a value of another type.
---*/

class A {
    fld: string = "a"
}

class B extends A {}

function main(): void {
  let b: B = new B()
  b.fld = "b"

  let a = b as A
  arktest.assertTrue( a instanceof A )
  arktest.assertTrue( a instanceof B )
  arktest.assertTrue( a.fld == "b" )

  let o = a as Object
  arktest.assertTrue( o instanceof A )
  arktest.assertTrue( o instanceof B )
  arktest.assertTrue( o instanceof Object )

  let a2 = o as A
  arktest.assertTrue( a2 instanceof A )
  arktest.assertTrue( a2 instanceof B )
  arktest.assertTrue( a2.fld == "b" )

  let b2 = o as B
  arktest.assertTrue( b2 instanceof A )
  arktest.assertTrue( b2 instanceof B )
  arktest.assertTrue( b2.fld == "b" )
}
