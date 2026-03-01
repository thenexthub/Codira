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
    The record indexing expression can be written by using a field access notation
    if its index expression is formed as an identifier of type string.
---*/

type Keys = 'key1' | 'key2' | 'key3'

let x: Record<Keys, number> = {
    'key1': 1,
    'key2': 2,
    'key3': 4,
}

function main(): void {
  arktest.assertEQ(x.{{c.index}}, {{c.value}});
}
{% endfor %}
