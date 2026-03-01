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
    This approach is based on the concept of truthiness that extends the Boolean
    logic to operands of non-Boolean types, while the result of an operation
    is kept boolean.
---*/
enum Color { Red, Green, Blue }
enum Size { S, M, L }

let a: {{a.type}} = {{a.val}};  // {% if a.truthy %}truthy{% else %}falsy{% endif %}

function main() {
    arktest.assertTrue(!a ? {% if a.truthy %}false : true{% else %}true : false{% endif %});

{%- for b in cases %}
{%- if a != b %}
    {
        let b: {{b.type}} = {{b.val}};  // {% if b.truthy %}truthy{% else %}falsy{% endif %}
{%- if a.truthy %}
        arktest.assertTrue((a && b) ? {% if b.truthy %}true : false{% else %}false : true{% endif %});
        arktest.assertTrue((a || b) ? true : false);
{%- else %}
        arktest.assertTrue((a && b) ? false : true);
        arktest.assertTrue((a || b) ? {% if b.truthy %}true : false{% else %}false : true{% endif %});
{%- endif %}
    }
{%- endif %}
{%- endfor %}
}
{%- endfor %}
