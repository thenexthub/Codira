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
    If the type used in the context is a readonly array,
    then an array literal is of readonly array type.
---*/

function bar (tuple: [Number, Boolean]) {
    arktest.assertEQ(tuple[0], 1.0)
    arktest.assertEQ(tuple[1], true)
}

function barReadonly (tuple: readonly [Number, Boolean]) {
    arktest.assertEQ(tuple[0], 1.0)
    arktest.assertEQ(tuple[1], true)
}

function fooReadonly (ar: readonly number[]) {
    arktest.assertEQ(ar[0], 1.0)
}

function main() {
    let tuple: [Number, Boolean] = [1, true] // assignment context
    arktest.assertEQ(tuple[0], 1.0)
    arktest.assertEQ(tuple[1], true)

    bar([1, true])  // call context

    let t: [Number, Boolean] = [1, true] as [Number, Boolean] // casting conversion
    arktest.assertEQ(t[0], 1.0)
    arktest.assertEQ(t[1], true)

    let readonly_tuple: readonly [number, boolean] = [1, true]
    barReadonly(readonly_tuple)

    let readonly_array: readonly number[] = [1, 2, 3]
    fooReadonly(readonly_array)
    fooReadonly([1.0, 2.0])

}
