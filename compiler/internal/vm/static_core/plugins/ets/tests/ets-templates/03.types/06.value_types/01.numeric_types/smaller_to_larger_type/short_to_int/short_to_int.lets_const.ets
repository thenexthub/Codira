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
    These tests are designed to verify the possibility and correctness of converting values from the short type to the int type. 
    The tests cover various cases, such as negative and positive numbers, values in different number systems (decimal, octal, binary, hex), 
    type aliases (Short, Int), value is zero, constructor initialization (new Short(value)), specifying a plus sign before a value, 
    visually splitting the number with underscore sign, using the "to<Type>()" method, as well as the maximum and minimum values for the short type.
    Various options for declaring and initializing initial and resulting variables are considered, such as -> 
    "0. Variables (initial and result) are declared inside the function using the 'let' keyword.", 
    "1. Variables (initial and result) are declared inside the function using the 'const' keyword.", 
    "2. The initial variable is declared outside the function using the 'let' keyword, and the resulting variable is declared inside the function using the 'const' keyword",
    "3. Variables (initial and result) are initilized outside the function. The initial variable is declared using the 'const' keyword, and the resulting variable is declared using the 'let' keyword.",
    "4. Variables (initial and result) are initilized inside the function. The initial variable is declared using the 'let' keyword, and the resulting variable is declared using the 'const' keyword.".
---*/

{%- for data in testData %}
// {{data.desc}}
{%- set name_initial_variable0 = 'initial_0_' + (loop.index0)|string %} {# Name of initial variable #}
{%- set name_result_variable0 = 'result_0_' + (loop.index0)|string %} {# Name of result variable #}
function test_inside_lets_{{loop.index0}}(){
    let {{name_initial_variable0}}: {{data.initialType}} = {{data.value}}
    let {{name_result_variable0}}: {{data.resultType}} = {{name_initial_variable0}}
    arktest.assertTrue({{name_result_variable0}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable0}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertEQ({{name_result_variable0}}, {{data.result}});
}

{%- set name_initial_variable1 = 'initial_1_' + (loop.index0)|string %} {# Name of initial variable #}
{% set name_result_variable1 = 'result_1_' + (loop.index0)|string -%} {# Name of result variable #}
function test_inside_consts_{{loop.index0}}(){
    const {{name_initial_variable1}}: {{data.initialType}} = {{data.value}}
    const {{name_result_variable1}} = {{name_initial_variable1}}.to{{data.resultType|capitalize}}() 
    arktest.assertTrue( {{name_result_variable1}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable1}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertEQ({{name_result_variable1}}, {{data.result}});
}

{%- set name_initial_variable2 = 'initial_2_' + (loop.index0)|string %} {# Name of initial variable #}
{% set name_result_variable2 = 'result_2_' + (loop.index0)|string -%} {# Name of result variable #}
let {{name_initial_variable2}}: {{data.initialType}} = {{data.value}}
function test_outside_let_inside_const_{{loop.index0}}(){
    const {{name_result_variable2}}: {{data.resultType}} = {{name_initial_variable2}}
    arktest.assertTrue( {{name_result_variable2}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable2}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertEQ({{name_result_variable2}}, {{data.result}});
}

{%- set name_initial_variable3 = 'initial_3_' + (loop.index0)|string %} {# Name of initial variable #}
{% set name_result_variable3 = 'result_3_' + (loop.index0)|string -%} {# Name of result variable #}
const {{name_initial_variable3}}: {{data.initialType}} = {{data.value}}
let {{name_result_variable3}}: {{data.resultType}} = {{name_initial_variable3}}
function test_outside_const_let_{{loop.index0}}(){
    arktest.assertTrue( {{name_result_variable3}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable3}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertEQ({{name_result_variable3}}, {{data.result}});
}

{%- set name_initial_variable4 = 'initial_4_' + (loop.index0)|string %} {# Name of initial variable #}
{% set name_result_variable4 = 'result_4_' + (loop.index0)|string -%} {# Name of result variable #}
function test_inside_let_const_{{loop.index0}}(){
    let {{name_initial_variable4}} = ({{data.value}}).to{{data.initialType|capitalize}}() 
    const {{name_result_variable4}}: {{data.resultType}} = {{name_initial_variable4}}
    arktest.assertTrue({{name_result_variable4}} instanceof {{data.resultType}}, "The type of the actual result (" + (typeof {{name_result_variable4}}) + ") is of a different type than expected ({{data.resultType}})")
    arktest.assertEQ({{name_result_variable4}}, {{data.result}});
}
//-------------
{% endfor%}

{# Testsuite run #}
function main(): int {
    let testSuite = new arktest.ArkTestsuite('short_to_int.lets_consts');
    {% for d in testData %}
    testSuite.addTest('test_inside_lets_{{loop.index0}}', test_inside_lets_{{loop.index0}})
    testSuite.addTest('test_inside_consts_{{loop.index0}}', test_inside_consts_{{loop.index0}})
    testSuite.addTest('test_outside_let_inside_const_{{loop.index0}}', test_outside_let_inside_const_{{loop.index0}})
    testSuite.addTest('test_outside_const_let_{{loop.index0}}', test_outside_const_let_{{loop.index0}})
    testSuite.addTest('test_inside_let_const_{{loop.index0}}', test_inside_let_const_{{loop.index0}})
    {% endfor %}
    let res = testSuite.run();
    return res;
}