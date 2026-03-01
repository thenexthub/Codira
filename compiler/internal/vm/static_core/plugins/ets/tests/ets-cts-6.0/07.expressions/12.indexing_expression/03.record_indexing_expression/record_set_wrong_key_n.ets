/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

{% for c in cases %}
/*---
desc: >-
    For a Record<Key, Value> indexing, the index expression must be of type Key.
tags: [compile-only, negative]
---*/

let r: Record<'11' | '12' | '13', {{c.type}}> = {
    '11': 1,
    '12': 2,
    '13': 3
}

function foo(s: string): string {
    return s + '2';
}

function main() {
    r[{{c.call}}] = 10; // CTE expected
}
{% endfor %}
