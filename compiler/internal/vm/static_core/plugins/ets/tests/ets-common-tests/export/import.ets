/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
files:
- ./selective_export.ets
---*/

import { a, foo, bar, COPT} from './selective_export'

function foo(i: Required<COPT>) {
    let v: int = 0;
    if (i.j != undefined) {
        v += 1
    }
    if (i.i != 0) {
        v += 2;
    }
    return v;
}

function main(): void {
    let sum: int = a + foo() + bar();
    const result: int = 7;
    arktest.assertEQ(sum, result)
    arktest.assertEQ( foo( {i: 1.9, j: 'some str'} ), 3 )
}
