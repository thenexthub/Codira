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

 /*---
 desc: >-
    If lhsExpression is a record access expression (see Record Indexing Expression), possibly enclosed in parentheses, then:
    1. Object reference subexpression of lhsExpression is evaluated. If this evaluation completes abruptly, then so
    does the assignment expression. In that case, rhsExpression and the index subexpression are not evaluated,
    and no assignment occurs.
    2. If the evaluation completes normally, the index subexpression of lhsExpression is evaluated. If this eval-
    uation completes abruptly, then so does the assignment expression. In that case, rhsExpression is not
    evaluated, and no assignment occurs.
    3. If the evaluation completes normally, rhsExpression is evaluated. If this evaluation completes abruptly,
    then so does the assignment expression. In that case, no assignment occurs.
    4. Otherwise, the value of the index subexpression is used as the key, and the value of rhsExpression con-
    verted to the type of the record value is used as the value. In that case, the assignment results in storing
    the key-value pair in the record instance.

    {{c.desc}}
 ---*/

let result: string = ""
let map: Record<string, number> = {
    'John': 25,
    'Mary': 21,
}

function getRecord(): Record<string, number> {
    result = result + "getRecord "
    {{c.record|indent}}
    return map
}

function getIndex(ind: string): string {
    result = result + "getIndex "
    {{c.index|indent}}
    return ind
}

function getValue(val: number): number {
    result = result + "getValue "
    {{c.value|indent}}
    return val
}

function main() {
    result = "";
    {{c.main|indent}}
    arktest.assertEQ(result, {{c.assert_result}})
}
{% endfor %}