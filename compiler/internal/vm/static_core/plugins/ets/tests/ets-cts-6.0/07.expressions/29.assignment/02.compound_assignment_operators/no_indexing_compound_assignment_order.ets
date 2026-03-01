/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

{% for c in cases %}
  /*---
  desc: >-
     An assignment expression can be evaluated at runtime in one of the following ways:
     1. If lhsExpression is not an indexing expression

     • lhsExpression is evaluated to produce a variable. If the evaluation completes abruptly, then so does the
     assignment expression. In that case, rhsExpression is not evaluated, and no assignment occurs.
     • If the evaluation completes normally, then the value of lhsExpression is saved, and rhsExpression is eval-
     uated. If the evaluation completes abruptly, then so does the assignment expression. In that case, no
     assignment occurs.
     • If the evaluation completes normally, then the saved value of the left-hand-side variable, and the value of
     rhsExpression are used to perform the binary operation as indicated by the compound assignment operator.
     If the operation completes abruptly, then so does the assignment expression. In that case, no assignment
     occurs.
     • If the evaluation completes normally, then the result of the binary operation converts to the type of the
     left-hand-side variable. The result of such conversion is stored into the variable.

  ---*/


let result: string = ""
let test_object: A = new A()

class A {
    public fld: int = -1
}

function getObj(): A {
    result = result + "getObj "
    {{c.get_object|indent}}
    return test_object
}

function getValue(): int {
    result = result + "getValue "
    {{c.value|indent}}
}

function main() {
    result = "";  // reset due to jit mode warmup

    {{c.main|indent}}

    arktest.assertEQ(result, {{c.assert_result}})
}
{% endfor %}