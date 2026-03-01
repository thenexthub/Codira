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

{% for modif in ['let', 'const'] %}
{%- for data in testData %}
/*---
desc: Checking that the values of the larger type include all values from the smaller types. 
    These tests are designed to verify the possibility and correctness of converting values from the float type to the double type. 
    The variable is assigned values of selected list inside the function.
    {{data.desc}}.
    Modificator = {{modif}}
---*/

{%- set name_initial_variable0 = 'initial_' + (loop.index0)|string %} {# Name of initial variable #}
{%- set name_result_variable0 = 'result_' + (loop.index0)|string %} {# Name of result variable #}
function main(){
    {{modif}} {{name_initial_variable0}}: {{data.initialType}} = {{data.value}}
    {{modif}} {{name_result_variable0}}: {{data.resultType}} = {{name_initial_variable0}}
    arktest.assertTrue({{name_result_variable0}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable0}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertDoubleEQ({{name_result_variable0}}, {{data.result}}, {{data.absError}});
}

{% endfor%}
{% endfor%}