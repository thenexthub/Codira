/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
    Checking static field declaration in derived class
    with same name and same type as in superclass.
params: hiding with different type
tags: [compile-only, negative]
expected_out: {{c.expected_out}}
---*/

class Base {
  static field: {{c.base_type}} = {{c.base_value}}
}

class A extends Base {
  static field: {{c.derived_type}} = {{c.derived_value}}
}

function main(): void {
}

{% endfor %}
