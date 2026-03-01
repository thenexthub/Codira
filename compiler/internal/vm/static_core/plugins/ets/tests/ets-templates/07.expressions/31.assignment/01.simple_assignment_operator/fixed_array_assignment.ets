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
  If lhsExpression denotes indexing of fixed-size array, and the type of rhsExpression is not a subtype of
  array element type, then ArrayStoreError is thrown, and no assignment occurs.
  {{c.desc}}
params: abrupt completion, left-hand operand, static
---*/

class A {}
class B extends A {}

function assignAToArray(arrA: FixedArray<A>) {
    arrA[0] = new A()
}

function main() {
    {{c.main|indent}}
}
{% endfor %}