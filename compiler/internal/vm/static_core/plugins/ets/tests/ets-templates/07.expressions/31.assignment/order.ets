/*
 * Copyright (c) 2021 -2026 Huawei Device Co., Ltd.
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
    All assignment operators are syntactically right-associative (they group right-to-left).
    Thus, a=b=c means a=(b=c), which assigns the value of c to b and then assigns the value of b to a.
params: with and without braces
---*/

let result: String = ""

function getIdx(i: int): int {
    result += i
    return i
}

function main(): void {

	// without braces
	result = ""
	let x: int[] = new int[3]
	let t: int = x[getIdx(0)] = x[getIdx(1)] = x[getIdx(2)] = 3
	arktest.assertEQ(result, "012")
	arktest.assertEQ(t, 3)

	// with braces
	result = ""
	let y: int[] = new int[3]
	let q: int = (y[getIdx(0)] = (y[getIdx(1)] = (y[getIdx(2)] = 3)))
	arktest.assertEQ(result, "012")
	arktest.assertEQ(q, 3)
}
