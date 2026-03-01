/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
  If the left-hand operand expression is a field access expression e.f (§7.11), possibly enclosed in one or more pairs of parentheses, then:
  - lhsExpression e is evaluated: if the evaluation of e completes abruptly, then so does the assignment expression.
  - rhsExpression is evaluated: if the evaluation completes abruptly, then so does the assignment expression.
  - If that evaluation completes normally, then the value of rhsExpression is converted to the type of the field.
    In that case, the result of the conversion is assigned to the field.
  {{c.desc}}
params: normal completion, instance
tags: [no-warmup]
---*/

let result: string = ""

function computeField(): string {
    result = result + "computeField "
    {{c.compute_field}}
}

class A {
    {{c.field}}
    public constructor() {
        result = result + "classConstructor "
        {{c.class_constructor}}
    }
}

function getObj(): A {
    result = result + "getObj "
    return new A()
}

function getValue(): string {
    result = result + "getValue "
    {{c.value}}
}

function main() {
    result = "";  // reset due to jit mode warmup

    {{c.main|indent}}

    arktest.assertEQ(result, {{c.assert_result}})
    arktest.assertEQ(s, {{c.assert_s}})
}
{% endfor %}