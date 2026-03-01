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
desc: A compile-time error occurs, if a qualified name refers to a function with overload signature.
tags: [compile-only, negative]
---*/

class A {
  bar (i: int): string
  bar (n: number): string
  bar(...a: Any[]): Any {
    return 'bar';
  }

  foo1 (i: int): string {
    return 'foo1';
  }

  overload test { {{c.list}} }
}

function main(): void {
  let a: A = new A();
  a.test(1);
}

{% endfor %}
