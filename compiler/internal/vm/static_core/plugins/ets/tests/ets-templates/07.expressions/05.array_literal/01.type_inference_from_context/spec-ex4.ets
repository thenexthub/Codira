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
    There are cases when every array or tuple element of value type is converted
    to the type of the context that creates a new array or tuple.
---*/

function foo (array: Number[]) {
    arktest.assertEQ(array[0], 1.0)
    arktest.assertEQ(array[1], 2.0)
    arktest.assertEQ(array[2], 3.0)
}

function main() {
    let array: Number[] = [1, 2, 3] // assignment context
    arktest.assertEQ(array[0], 1.0)
    arktest.assertEQ(array[1], 2.0)
    arktest.assertEQ(array[2], 3.0)

    foo([1, 2, 3])  // call context

    let n: Number[] = [1, 2, 3] as Number[] // casting conversion
    arktest.assertEQ(n[0], 1.0)
    arktest.assertEQ(n[1], 2.0)
    arktest.assertEQ(n[2], 3.0)
}
