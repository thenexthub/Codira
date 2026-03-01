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
    The result of the field access expression is value or variable of the static field in the class or interface that is the type
    of the object reference expression:
    For a static field: the result of an object reference expression evaluation is discarded.
---*/

let m: int;

class A {
  static a: int;
  constructor() {
    A.a = 1;
  }
}

function main(): void {
  if( m == 0) {
    arktest.assertTrue( A.a == 0 )
    m++;
  }
  let i: A = new A();
  arktest.assertTrue( A.a == 1 )
  arktest.assertTrue( m == 1 )
}
