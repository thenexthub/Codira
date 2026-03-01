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
    Any entity can be compared to null by using the operators "==" and "===". This comparison can return true only
    for the entities of nullundef types if they actually have the null value during program execution. In all other cases the
    comparison to null returns false.
---*/

{{c.decl}}
let nullundef: null|undefined|{{c.type}} = {{c.value}}

function main(): void {
    arktest.assertTrue(nullundef == {{c.value2 if c.value2 else c.value}})
    arktest.assertFalse(nullundef != {{c.value2 if c.value2 else c.value}})
    arktest.assertTrue(nullundef === {{c.value2 if c.value2 else c.value}})
    arktest.assertFalse(nullundef !== {{c.value2 if c.value2 else c.value}})
}
{% endfor %}