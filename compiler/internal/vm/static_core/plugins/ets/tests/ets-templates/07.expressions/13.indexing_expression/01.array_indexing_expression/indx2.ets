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
    If the chaining operator ‘?.’ is present, and after its application the type of object reference
    expression is an array type, then it makes a valid array reference expression, and the type of the array indexing
    expression is T.
    If the value of the expression to the left of ‘?.’ is undefined or null, then the evaluation of the entire surrounding
    primary expression stops. The result of the entire primary expression is then undefined.
---*/

function refNullish(): string[]|{{c.type}} {
    let res: string[]|{{c.type}} = {{c.value}};
    return res;
}

function main() {
    arktest.assertEQ({{c.expected}}, refNullish()?.[0]);
}
{% endfor %}
