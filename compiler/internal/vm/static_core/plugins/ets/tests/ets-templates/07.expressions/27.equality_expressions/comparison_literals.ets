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

{% for ic in literals_comparison %}

/*---
desc: comparison with literals - {{ic.type}} literal
---*/

function main(): void {
    //check decimal, octal, hex and binary literals
    arktest.assertEQ({{ic.value1}} == {{ic.value2}}, true)
    arktest.assertEQ({{ic.value1}} === {{ic.value2}}, true)
    
    //underscore brings value except decimal! 0xBAD32 vs 0xBAD32_1
    //non decimal
    arktest.assertEQ({{ic.value1}} != {{ic.valueWithUnderscore}}, true)
    arktest.assertEQ({{ic.value1}} !== {{ic.valueWithUnderscore}}, true)
    //decimal
    arktest.assertEQ({{ic.value2}} != {{ic.valueWithUnderscore}}, true)
    arktest.assertEQ({{ic.value2}} !== {{ic.valueWithUnderscore}}, true)

    //check literals with underscore i.e. 1153 == 1_153
    arktest.assertEQ({{ic.valueWithUnderscore}} == {{ic.underscoreCheck}}, true)
    arktest.assertEQ({{ic.valueWithUnderscore}} === {{ic.underscoreCheck}}, true)
}

{% endfor %}
