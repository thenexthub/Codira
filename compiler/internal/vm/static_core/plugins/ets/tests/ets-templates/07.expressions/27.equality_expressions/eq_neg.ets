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

{%- for index1 in range( types|length ) %}
    {%- for index2 in range( (index1 + 1), types|length ) %}
        {%- for t1 in types[index1].types %}
            {%- for t2 in types[index2].types %}
                {%- for op in operators %}

/*---
desc: |-
    if types A and B do not overlap (and therefore an expression always evaluated to false at compile time), then:
    - if each of A and B is either a predefined type or a union of predefined types, a compile-time-error is issued.
tags: [compile-only, negative]
---*/

let v: boolean = false

function compare(a: {{t1}}, b: {{t2}}) {
    v = a {{op}} b;
}
                {%- endfor %}
            {%- endfor %}
        {%- endfor %}
    {%- endfor %}
{%- endfor %}
