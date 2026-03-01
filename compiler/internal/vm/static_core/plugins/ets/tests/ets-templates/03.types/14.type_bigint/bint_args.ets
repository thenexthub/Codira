/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
    Numeric type values, by using a call to the standard library
    (see Standard Library) class BigInt methods or constructors.
---*/
function foo(p: bigint): bigint {
    return p + 1n
}

function main() {
{%- for t in c['values'] %}
    let prim{{loop.index}}: {{c.from_type}} = {{t.expr|safe}}
    arktest.assertTrue( foo(new BigInt(prim{{loop.index}})) == {{t.val}} + 1n)
{%- endfor %}
}
{%- endfor %}
