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
{% for t in c['types'] %}
/*---
desc: Any instanceof expression in the form 'expr instanceof T' is of type boolean
tags: [compile-only, negative]
---*/

{{c.decl}}

function main() {
{%- if t.res == 'rte' %}
    arktest.expectThrow(() => {({{c.expr}} instanceof {{t.type}})})
{%- else %}
    let v1 = {{c.expr}} instanceof {{t.type}}
    let v2 = ((p: boolean) => p ? 1 : 0)({{c.expr}} instanceof {{t.type}})
{%- endif %}
}
{% endfor %}
{% endfor %}
