/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{% for s in syntax %}
{% for c in cases %}
/*---
desc: |-
    The result of an instanceof expression is true if the actual type of evaluated expr is a subtype of T.
    Otherwise, the result is false. Generic type in the form of type name can be used as T operand of an
    instanceof expression. In this case, the check is performed against the type name, and type parameters
    are ignored.
---*/

{{c.decl}}
{%- if s == 'explicit' %}
let a: {{c.type}} = {{c.init}}
{%- else %}
let a = {{c.init}}
{%- endif %}

function pass_through(value: Any): Any { return value }

function main() {
    let v = pass_through(a)
    arktest.assertTrue(v instanceof Array, "failed at Array of type {{c.type}} {{s.syntax}}")
    arktest.assertTrue(a[0] instanceof {{c.effective_elem_type}},
       "failed at elem of type {{c.effective_elem_type}} Array of type {{c.type}} {{s.syntax}}")
}
{% endfor %}
{% endfor %}
