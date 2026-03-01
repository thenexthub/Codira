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
desc: >-
    If class T has a user-defined getter, setter, or both, then none of those is called when object literal is used with
    Partial<T> variables. Object literal has its own built-in getters and setters to modify its variables. It is represented
    in the example below:
---*/

let i: int = 0

interface I {
  property: number
}

class A implements I {
  set property(property: number) { i+=1 }
  get property(): number { i+=10; return this.property }
}

function foo (partial: Partial<A>) {
  partial.property = 42 // setter to be called
  return partial.property // getter to be called
}

function main() {
  arktest.assertEQ( foo({property: 1}), 42) // No getter or setter from class A is called
  // 42 is printed as object literal has its own setter and getter
  arktest.assertEQ( i, 0 )
}