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
{% for array in arrays %}
  /*---
  desc: >-
     If lhsExpression is an array reference expression (see Array Indexing Expression), then:
     • Array reference subexpression of lhsExpression is evaluated. If the evaluation completes abruptly, then so
     does the assignment expression. In that case, the index subexpression, and rhsExpression are not evaluated,
     and no assignment occurs.
     • If the evaluation completes normally, then the index subexpression of lhsExpression is evaluated. If the
     evaluation completes abruptly, then so does the assignment expression. In that case, rhsExpression is not
     evaluated, and no assignment occurs.
     • If the evaluation completes normally, the value of the array reference subexpression refers to an array, and
     the value of the index subexpression is less than zero, greater than, or equal to the length of the array, then
     RangeError is thrown. In that case, no assignment occurs.
     • If the evaluation completes normally, then the value of the index subexpression is used to select an array
     element referred to by the value of the array reference subexpression. The value of this element is saved,
     and then rhsExpression is evaluated. If the evaluation completes abruptly, then so does the assignment
     expression. In that case, no assignment occurs.
     • If the evaluation completes normally, consideration must be given to the saved value of the array element
     selected in the previous step. While this element is a variable of type S, and T is type of lhsExpression of
     the assignment operator determined at compile time:
     – If T is a predefined value type, then S is the same as T.
     The saved value of the array element, and the value of rhsExpression are used to perform the binary
     operation of the compound assignment operator.
     If this operation completes abruptly, then so does the assignment expression. In that case, no assign-
     ment occurs.
     If this evaluation completes normally, then the result of the binary operation converts to the type of
     the selected array element. The result of the conversion is stored into the array element.
     – If T is a reference type, then it must be string.
     S must also be a string because the class string is the final class. The saved value of the array
     element, and the value of rhsExpression are used to perform the binary operation (string concatena-
     tion) of the compound assignment operator ‘+=’. If this operation completes abruptly, then so does
     the assignment expression. In that case, no assignment occurs.
     – If the evaluation completes normally, then the string result of the binary operation is stored into
     the array element.
  ---*/


let result: string = ""
let test_array: {{array.array_type}} = [1, 2]

function getArray(): {{array.array_type}} {
    result = result + "getArray "
    if ({{c.array or 'false'}})
        throw new Error("Any error to break assignment")
    return test_array
}

function getIndex(ind: int): int {
    result = result + "getIndex "
    if ({{c.index or 'false'}})
        throw new Error("Any error to break assignment")
    return ind
}


function getValue(val: long): long {
    result = result + "getValue "
    if ({{c.value or 'false'}})
        throw new Error("Any error to break assignment")
    return val
}

function main() {
    result = "";  // reset due to jit mode warmup
    test_array = [1, 2]
    {{c.main|indent}}

    arktest.assertEQ(result, {{c.assert_result}})
}
{% endfor %}
{% endfor %}