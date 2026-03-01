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
  If the lhsExpression is an array reference expression (see Array Indexing Expression), possibly enclosed in
  parentheses
  1. Array reference subexpression of lhsExpression is evaluated. If this evaluation completes abruptly, then so
  does the assignment expression. In that case, rhsExpression and the index subexpression are not evaluated,
  and no assignment occurs.
  2. If the evaluation completes normally, then the index subexpression of lhsExpression is evaluated. If this
  evaluation completes abruptly, then so does the assignment expression. In that case, rhsExpression is not
  evaluated, and no assignment occurs.
  3. If the evaluation completes normally, then rhsExpression is evaluated. If this evaluation completes
  abruptly, then so does the assignment expression, and no assignment occurs.
  4. If the evaluation completes normally, but the value of the index subexpression is less than zero, or greater
  than, or equal to the length of the array, then RangeError is thrown, and no assignment occurs.
  5. If lhsExpression denotes indexing of fixed-size array, and the type of rhsExpression is not a subtype of
  array element type, then ArrayStoreError is thrown, and no assignment occurs.
  6. Otherwise, the value of the index subexpression is used to select an element of the array referred to by the
  value of the array reference subexpression and the value of rhsExpression is converted to the type of the
  array element. In that case, the result of the conversion is assigned to the array element.

  {{c.desc}}
params: abrupt completion, left-hand operand, static
tags: [no-warmup]
---*/

let result: string = ""
let arr: string[] = new string[3]

function getArray(): string[] {
    result = result + "getArray "
    {{c.array}}
}

function getIndex(i: int): int {
    result = result + "getIndex "
    {{c.index}}
}

function getValue(): string {
    result = result + "getValue "
    {{c.value}}
}

function main() {
    result = "";

    {{c.main|indent}}

    arktest.assertEQ(result, {{c.assert_result}})
    arktest.assertEQ(s, {{c.assert_s}})
}
{% endfor %}