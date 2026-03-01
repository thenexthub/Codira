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
    A function reference refers to a declared or imported function.
    Type of a function reference is derived from the function signature
files:
- ./function_reference_lib.ets
---*/

import {sum} from "./function_reference_lib.ets"

function fun1(): string {
    return 'ok';
}

function fun2(i: int): undefined {
    return undefined;
}

function funGeneric<T> (x: T): T {
    return x;
}

function main() {

    let ref1 = fun1;
	arktest.assertEQ(ref1(), 'ok')

	let ref2 = fun2;
	arktest.assertEQ(ref2(1), undefined)

	let ref3 = sum;
	arktest.assertEQ(ref3(1, 2), 3 as int)

	let ref4 = ref1;
	arktest.assertEQ(ref4(), 'ok')

	let refGeneric = funGeneric<string> // ; #28174
	arktest.assertEQ(refGeneric('yes'), 'yes')
  
}