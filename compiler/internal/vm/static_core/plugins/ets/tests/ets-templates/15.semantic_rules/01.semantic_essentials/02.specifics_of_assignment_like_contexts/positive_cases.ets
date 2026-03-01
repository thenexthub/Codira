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

/*---
desc: >-
    Assignment-like context is specific in that the type of a left-hand-side expression is known,
    but the type of a right-hand-side expression is not necessarily known in the context as follows
---*/

class A {}

class B extends A {}

{%- for c in cases %}
function assignSameType{{loop.index}}(x: {{c.type1}}, y: {{c.type1}}) {
    x = y
}
function assignOtherType{{loop.index}}(x: {{c.type1}}, y: {{c.type2}}) {
    x = y {{c.typeguard}}
}
function assignIndexed{{loop.index}}(x: {{c.type1}}[]) {
    x = [{{c.val1}}, {{c.val2}}]
}
{%- endfor %}

function main() {
{%- for c in cases %}
    assignSameType{{loop.index}}({{c.val1}}, {{c.val1}})
    assignOtherType{{loop.index}}({{c.val1}}, {{c.val2}})
    assignIndexed{{loop.index}}([])
{%- endfor %}
}
