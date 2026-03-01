/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}

/*---
desc: >-
    A class instance creation expression that refers to classes FixedArray, Array, or classes derived from Array,
    instantiated with an array element type of some class type, is a special form of array creation expression.
    When defining multiple elements of a created array, such an array creation expression must:
    - Refer to a class that contains an accessible parameterless constructor or a constructor with
    all parameters of the second form of optional parameters (see :ref:Optional Parameters); or
    - Have a default value.

    Otherwise, a compile-time error occurs.
---*/

enum ENUM { Red, Green, Blue }
type TPL = [number, string]

class A<T> extends Array<T> {
  i ?: {{c.type}}
  constructor({{c.param}}) {
    this.i = i
  }
}

class C<T> {
  foo() {
    let arr = new Array<T>(5)
    arktest.assertEQ(arr.length, 5)
  }
}

function main(): void {
  new C<A<Object>>().foo()
}

{% endfor %}
