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

/*---
desc: Checking that the values of the larger type include all values from the smaller types. 
    These tests are designed to verify the possibility and correctness of converting values from the float type to the double type. 
    The initial variable is initialized the outside the function.   
---*/

{%- for data in testData %}
// {{data.desc}}
{%- set name_initial_variable2 = 'initial_2_' + (loop.index0)|string %} {# Name of initial variable #}
{% set name_result_variable2 = 'result_2_' + (loop.index0)|string -%} {# Name of result variable #}
let {{name_initial_variable2}}: {{data.initialType}} = {{data.value}}
function test_outside_let_inside_const_{{loop.index0}}(){
    const {{name_result_variable2}}: {{data.resultType}} = {{name_initial_variable2}}
    arktest.assertTrue( {{name_result_variable2}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable2}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertDoubleEQ({{name_result_variable2}}, {{data.result}}, {{data.absError}});
}

{%- set name_initial_variable3 = 'initial_3_' + (loop.index0)|string %} {# Name of initial variable #}
{% set name_result_variable3 = 'result_3_' + (loop.index0)|string -%} {# Name of result variable #}
const {{name_initial_variable3}}: {{data.initialType}} = {{data.value}}
let {{name_result_variable3}}: {{data.resultType}} = {{name_initial_variable3}}
function test_outside_const_let_{{loop.index0}}(){
    arktest.assertTrue( {{name_result_variable3}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable3}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertDoubleEQ({{name_result_variable3}}, {{data.result}}, {{data.absError}});
}
//-------------
{% endfor%}

{# Testsuite run #}
function main(): int {
    let testSuite = new arktest.ArkTestsuite('float_to_double.lets_consts');
    {% for d in testData %}
    testSuite.addTest('test_outside_let_inside_const_{{loop.index0}}', test_outside_let_inside_const_{{loop.index0}})
    testSuite.addTest('test_outside_const_let_{{loop.index0}}', test_outside_const_let_{{loop.index0}})
    {% endfor %}
    let res = testSuite.run();
    return res;
}