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

{% for union_type in union_with_type %}
{% for c in cases %}
/*---
desc: >-
    Literal types are aligned with some ArkTS literals (see Literals).
    Their names are same as the names of their values, i.e., literals.
    Only three literal types are supported - string, null and undefined.
tags: [negative, compile-only]
---*/

type number_tuple = [int, double]

class SomeClass {}

type fooType = () => int

{{c.decl}}

function main(): void {
    const {{c.test_type}} | {{union_type.def}} = {{c.rh_value}}
    arktest.assertTrue({{c.retrieve}} instanceof {{c.literal}})
    arktest.assertEQ({{c.retrieve}}, {{c.literal}})
}

{% endfor %}
{% endfor %}
