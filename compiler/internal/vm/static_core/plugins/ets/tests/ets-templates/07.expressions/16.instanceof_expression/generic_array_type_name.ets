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

{% for c in cases %}
{% for s in syntax %}
/*---
desc: |-
    T[] and Array<T> specify identical, i.e., indistinguishable types... Generic type in the form of type name can be
    used as T operand of an instanceof expression. In this case, the check is performed against the type name, and
    type parameters are ignored.
---*/

{{c.top_decl}}

{%- if s == 'square' %}
type my_type = {{c.type}}[]
{%- else %}
type my_type = Array<{{c.type}}>
{%- endif %}

function main() {
    {{c.decl}}
    let v: my_type = {{c.init}}
    arktest.assertTrue(v instanceof Array)
}
{% endfor %}
{% endfor %}
