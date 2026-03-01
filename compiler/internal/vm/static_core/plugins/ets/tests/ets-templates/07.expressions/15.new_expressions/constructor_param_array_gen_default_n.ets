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
    When defining multiple elements of a created array, a compile-time error occurs if there is any non-default
    or optional parameter in constructor.
tags: [compile-only, negative]
expected_out: {{c.error}}
---*/

{{c.decl}}

class A<T> extends Array<T> {
  constructor({{c.param}}) { }
}

class C<T> {
  foo() {
    let arr = new Array<T>(5)
  }
}

function main(): void {
  new C<A<Object>>().foo()
}

{% endfor %}
