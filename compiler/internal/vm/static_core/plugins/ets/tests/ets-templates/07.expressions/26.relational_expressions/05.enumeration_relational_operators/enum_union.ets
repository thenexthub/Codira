/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

{%- for c in cases %}
/*---
desc: >-
    If both operands are of the same Enumeration type, then Numerical Relational Operators
    or String Relational Operators are used depending on the kind of enumeration constant value.
---*/

{{c.decl}}
enum Size { S = "a", M = "b", L = "c", XL = "d" }
let r: Color|Size|string = Color.Red
let g: Size|Color = Color.Green
let b: Size|Color|int = Color.Blue

function main() {
{% for d in operands %}
    arktest.assertTrue({{c.op[0]}}({{d[0]}} < {{d[1]}}))
    arktest.assertTrue({{c.op[1]}}({{d[0]}} < {{d[2]}}))
    arktest.assertTrue({{c.op[2]}}({{d[1]}} < {{d[2]}}))
    arktest.assertTrue({{c.op[3]}}({{d[0]}} <= {{d[1]}}))
    arktest.assertTrue({{c.op[4]}}({{d[0]}} <= {{d[2]}}))
    arktest.assertTrue({{c.op[5]}}({{d[1]}} <= {{d[2]}}))
    arktest.assertTrue({{c.op[6]}}({{d[0]}} > {{d[1]}}))
    arktest.assertTrue({{c.op[7]}}({{d[0]}} > {{d[2]}}))
    arktest.assertTrue({{c.op[8]}}({{d[1]}} > {{d[2]}}))
    arktest.assertTrue({{c.op[9]}}({{d[0]}} >= {{d[1]}}))
    arktest.assertTrue({{c.op[10]}}({{d[0]}} >= {{d[2]}}))
    arktest.assertTrue({{c.op[11]}}({{d[1]}} >= {{d[2]}}))
{% endfor %}
}
{%- endfor %}
