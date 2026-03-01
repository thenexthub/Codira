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

{%- for c in cases %}
/*---
desc: >-
    Otherwise, an attempt is made to apply the type of the left-hand-side expression to the right-hand-side expression.
    A compile-time error occurs if the attempt fails as in the example below
tags: [compile-only, negative]
expected_out: {{c.stdout}}
---*/

function assign(x: {{c.type1}}, y: {{c.type2}}) {
    x = y
}

function main() {
    assign({{c.val1}}, {{c.val1}})
}
{%- endfor %}
