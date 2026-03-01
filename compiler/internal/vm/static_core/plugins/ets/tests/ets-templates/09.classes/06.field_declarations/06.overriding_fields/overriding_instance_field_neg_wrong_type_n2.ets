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
    The type of the overriding field is to be the same as that of the
    overridden field. Otherwise, a compile-time error occurs
tags: [compile-only, negative]
expected_out: {{c.expected_out}}
---*/

class Animal {}

class Dog extends Animal {}

class Cat extends Animal {}

class Base {
  field: {{c.base_type}} = new {{c.base_type}}
}

class A extends Base {
  field: {{c.override_type}} = new {{c.override_type}}
}

function main(): void {
}

{% endfor %}
