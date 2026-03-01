{%- for a in cases %}
/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    The actual extended semantics of conditional-and and conditional-or expressions
    is described in the following truth tables (assuming that 'A' and 'B' are any
    valid expressions)
---*/
enum Color { Red, Green, Blue }
enum Size { S, M, L }

let a: {{a.type}} = {{a.val}};  // {% if a.truthy %}truthy{% else %}falsy{% endif %}

function main() {
    arktest.assertTrue(!a == {% if a.truthy %}false{% else %}true{% endif %});

{%- for b in cases %}
{%- if a != b %}
    {
        let b: {{b.type}} = {{b.val}};  // {% if b.truthy %}truthy{% else %}falsy{% endif %}
{%- if a.truthy %}
    {%- if b.nan %}
        arktest.assertTrue(Number.isNaN((a && b) as Number));
    {%- else %}
        arktest.assertTrue((a && b) == b);
    {%- endif %}
        arktest.assertTrue((a || b) == a);
{%- else %}
    {%- if a.nan %}
        arktest.assertTrue(Number.isNaN((a && b) as Number));
    {%- else %}
        arktest.assertTrue((a && b) == a);
    {%- endif %}
    {%- if b.nan %}
        arktest.assertTrue(Number.isNaN((a || b) as Number));
    {%- else %}
        arktest.assertTrue((a || b) == b);
    {%- endif %}
{%- endif %}
    }
{%- endif %}
{%- endfor %}
}
{%- endfor %}
