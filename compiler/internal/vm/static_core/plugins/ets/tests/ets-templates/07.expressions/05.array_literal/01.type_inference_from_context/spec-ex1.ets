/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
  The type of an array literal can be inferred from the context,
  including explicit type annotation of a variable declaration,
  left-hand part type of an assignment, call parameter type,
  or type of a cast expression.
---*/

function min(x: number[]): number {
    let m = x[0]
    for (let v of x)
        if (v < m)
            m = v
    return m
}

type Matrix = number[][]

function main() {
    let a: number[] = [1, 2, 3] // ok, variable type is used
    a = [4, 5] // ok, variable type is used
    arktest.assertEQ( min([1., 3.14, 0.99]), 0.99 ); // ok, parameter type is used

    let m: Matrix = [[1, 2], [3, 4], [5, 6]]
    let value: number = 2
    let list: Object[] = [1, value, "hello", new Error()] // ok
}
