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

{% for c in cases %}

/*---
desc: A constant declaration introduces a named entity with mandatory initial value
tags: [negative, compile-only]
---*/

type literal = "aa"
type tuple = [literal, int]
type tuple2 = [tuple, int]
type array = tuple[]
type fixed_array = FixedArray<tuple>
type fooType = (p: literal) => literal
function foo(p: literal): literal { return p }

{{c.decl}}

function main(): void {
    const some?{{c.type_annotation}} = {{c.initializer}}
    arktest.assertTrue(some{{c.retrieve}} instanceof {{c.expected_type}} )
}
{% endfor %}
