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
    The expression must be of array type (see Array Types),
    tuple type (see Tuple Types),
    or any type which has the iterator defined (see Iterable Types).
---*/

class A<T> implements Iterable<T>, Iterator<T> {
    data: T[]
    index = 0

    constructor(p: T[]) {
        this.data = p
    }

    $_iterator(): Iterator<T> {
        return this
    }

    next(): IteratorResult<T> {
        return {
            done: this.index >= this.data.length,
            value:
                this.index >= this.data.length
                    ? undefined
                    : this.data[this.index++],
        }
    }
}

let it: Iterable<double> = new A<double>([1.0, 2.0, 3.0])

function main() {
    let r: double[] = [...it]
    arktest.assertEQ(r.length, 3)
    arktest.assertEQ(r[0], 1.0)
    arktest.assertEQ(r[1], 2.0)
    arktest.assertEQ(r[2], 3.0)
}
