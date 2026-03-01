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
    Indexing expression evaluation order.
---*/

let res = ""

let rec: Record<string, number> = {
    "1" : 42
}

function get_ref(fail: boolean): Record<string, number> {
    res += " ref"
    if (fail) throw new Error();
    return rec
}

function get_idx(fail: boolean, idx: string): string {
    res += " idx"
    if (fail) throw new Error();
    return idx
}

let v: number|undefined;

function main() {
    res = ""
    v = 99

    try {
        v = get_ref({{c.ref_fail}})[get_idx({{c.idx_fail}}, {{c.idx}})]
        res += " passed " + v
    } catch (e) {
        res += " failed " + v
    }
    arktest.assertTrue(res == {{c.res}})
}
{% endfor %}
