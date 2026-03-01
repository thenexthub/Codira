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
    Methods with special names (see Indexable Types, Iterable Types, and Callable Types) can be overloaded like ordinary methods.
---*/

class C {
    private ids: Map<int, int> = new Map<int, int>();
    private names: Map<string, int> = new Map<string, int>();

    getByNumber(index: int): int|undefined { return this.ids.get(index) }
    getByString(index: string): int|undefined { return this.names.get(index) }
    overload $_get { getByNumber, getByString }

    setByNumber(index: int, value: int) { this.ids.set(index, value) }
    setByString(index: string, value: int) { this.names.set(index, value) }
    overload $_set { setByNumber, setByString }
}


function main() {
    let c = new C();

    c[0] = 1;
    c[1] = 10;
    c['a'] = 20;
    c[''] = 30;

    arktest.assertEQ({{c.value}}, c[{{c.param}}]);
}
{%- endfor %}
