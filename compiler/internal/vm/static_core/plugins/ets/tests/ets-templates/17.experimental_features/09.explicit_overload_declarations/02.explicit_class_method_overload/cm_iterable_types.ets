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
    data1: string[] = ['a', 'b', 'c'];
    data2: int[] = [2, 12, 45, 0, 6];
    iter1(): CIter1 { return new CIter1(this) }
    iter2(): CIter2 { return new CIter2(this) }
    overload $_iterator{ {{c.list}} }
}

class CIter1 implements Iterator<string> {
    index = 0;
    base: C;
    constructor (base: C) {
        this.base = base;
    }
    next(): IteratorResult<string> {
        return {
            done: this.index >= this.base.data1.length,
            value: this.index >= this.base.data1.length ? undefined : this.base.data1[this.index++]
        }
    }
}

class CIter2 implements Iterator<int> {
    index = 0;
    base: C;
    constructor (base: C) {
        this.base = base;
    }
    next(): IteratorResult<int> {
        return {
            done: this.index >= this.base.data2.length,
            value: this.index >= this.base.data2.length ? undefined : this.base.data2[this.index++]
        }
    }
}

function main() {
    let c = new C();
    let {{c.res}};
    for (let x of c) {
        res += x;
    }

    arktest.assertEQ({{c.value}}, res);
}
{%- endfor %}
