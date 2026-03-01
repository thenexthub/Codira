/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
    If lhsExpression is a record access expression (see Record Indexing Expression):
    • The object reference subexpression of lhsExpression is evaluated. If this evaluation completes abruptly,
    then so does the assignment expression. In that case, the index subexpression and rhsExpression are not
    evaluated, and no assignment occurs.
    • If this evaluation completes normally, then the index subexpression of lhsExpression is evaluated. If the
    evaluation completes abruptly, then so does the assignment expression. In that case, rhsExpression is not
    evaluated, and no assignment occurs.
    • If this evaluation completes normally, the value of the object reference subexpression and the value of
    index subexpression are saved, then rhsExpression is evaluated. If the evaluation completes abruptly, then
    so does the assignment expression. In that case, no assignment occurs.
    • If this evaluation completes normally, the saved values of the object reference subexpression and index
    subexpression (as the key) are used to get the value that is mapped to the key (see Record Indexing Expres-
    sion), then this value and the value of rhsExpression are used to perform the binary operation as indicated
    by the compound assignment operator. If the operation completes abruptly, then so does the assignment
    expression. In that case, no assignment occurs.
    • If the evaluation completes normally, then the result of the binary operation is stored as the key-value pair
    in the record instance (as in Simple Assignment Operator).

    {{c.desc}}
---*/

  let result: string = ""
  type Keys = 'John' | 'Mary'

  let map: Record<Keys, int> = {
      'John': 2,
      'Mary': -2,
  }


  function getRecord(): Record<Keys, int> {
    result = result + "getRecord "
    {{c.record|indent}}
    return map
  }

  function getIndex(ind: Keys): Keys {
    result = result + "getIndex "
    {{c.index|indent}}
    return ind
  }

  function getValue(val: int): int {
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