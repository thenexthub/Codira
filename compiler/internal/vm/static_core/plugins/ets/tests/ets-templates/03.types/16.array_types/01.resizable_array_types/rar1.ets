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
{% for t in cases %}
{%- if t != c %}
/*---
desc: >-
    Testing type erasure of Array<{{c.type}}> elements.
    Casting array to Object and then to Array<U> type.
    Expecting success on assigning U type value to array element and failure
    (ClassCastError) on retrieving '{{c.type}}' type value.
---*/
{{c.decl}}
{{t.decl}}

function toObj(p: Object|null|undefined): Object {
    return p!
}

let array: Array<{{c.type}}> = [{{c.vals}}]

function main() {
    let obj: Object = toObj(array)
    let arr: Array<{{t.type}}> = obj as Array<{{t.type}}>
    arr[0] = {{t.val}}
    let v: {{t.type}} = arr[0]
    try {
        v = arr[1]
        arktest.assertTrue(false, 'ClassCastError expected')
    } catch (e) {
        arktest.assertTrue(e instanceof ClassCastError, 'ClassCastError expected')
    }
    arktest.assertEQ(v, {{t.val}})
}
{%- endif %}
{% endfor %}
{% endfor %}
