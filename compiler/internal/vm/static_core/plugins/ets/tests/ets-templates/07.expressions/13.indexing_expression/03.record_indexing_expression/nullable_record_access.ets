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
    If no ‘?.’ is present in an indexing expression, then the type of object reference
    expression must be an array type, or the Record type; a compile-time error occurs otherwise.
---*/

function refNullish(): Record<string, number> | {{c.type}} {
    let res: Record<string, number> | {{c.type}} = {{c.value}};
    return res;
}

function main() {
    arktest.assertEQ(refNullish()?.["magic"], {{c.result}});
}
{% endfor %}
