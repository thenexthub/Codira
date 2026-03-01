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
desc: >-
    Type T is compatible (see Type Compatibility) with Readonly<T>, and allows assignments as a consequence.
    Cannot assign different types to the same variable using Readonly<T> though.
tags: [negative, compile-only]
---*/

interface I {
  i: number;
  j: string;
}

class A implements I {
  i = 0.1;
  j = "";
}

class C {
  a: number = 0.2;
  b: string = "";
}

function main() {
  let z: {{c.type}} = {{c.value}}
}

{% endfor %}
