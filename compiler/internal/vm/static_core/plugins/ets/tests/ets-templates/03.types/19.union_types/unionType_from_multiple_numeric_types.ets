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

/*---
name: unionType_from_multiple_numeric_types
desc: When a union type contains multiple numeric types, it will not cause a compile-time error..
---*/

{%- for t1 in types %}
{% set outer_loop_index = loop.index0 %}
{%- for t2 in types %}

{%if t1.upper() != t2.upper() %}
type UT{{outer_loop_index}}_{{loop.index0}} = {{t1}} | {{t2}}

function func{{outer_loop_index}}_{{loop.index0}}_unionType() {
   let test{{outer_loop_index}}_{{loop.index0}}: UT{{outer_loop_index}}_{{loop.index0}};
}
{%endif %}
{%- endfor %}
{%- endfor %}


{# Testsuite run #}
function main(): int {
    let testSuite = new arktest.ArkTestsuite('unionType_from_multiple_numeric_types');
    
    {% for t1 in types %}
    {% set outer_loop_index = loop.index0 %}
    {% for t2 in types %}

    {%- if t1.upper() != t2.upper() %}
    testSuite.addTest('{{t1}}_{{t2}}_unionType', func{{outer_loop_index}}_{{loop.index0}}_unionType);
    {%- endif %}

    {% endfor %}
    {% endfor %}

    return testSuite.run(); 
}