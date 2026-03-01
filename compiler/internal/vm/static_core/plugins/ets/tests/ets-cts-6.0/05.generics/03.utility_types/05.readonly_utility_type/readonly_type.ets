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

{% for c in cases %}

/*---
desc: Type Readonly<T> constructs a type with all properties of T set to readonly. It means that the properties of the
      constructed value cannot be reassigned. T must be a class or an interface type
tags: [{% if c.result == -1 %}compile-only, negative{% endif %}]
---*/

interface I {
  i: number;
  j: string;
}

class A implements I {
  i = 1.7;
  j = "A";
}

class C {
  i: number = 2.6;
  j: string = "C";
}

function foo(i: Readonly<A>|Readonly<C>, a: number, b: string) {
  arktest.assertEQ(i.i, a)
  arktest.assertEQ(i.j, b)
}

function main() {
  let i: Readonly<{{c.class}}> = {{c.argument}}
  foo( i, {{c.num}}, {{c.str}} )
}

{% endfor %}
