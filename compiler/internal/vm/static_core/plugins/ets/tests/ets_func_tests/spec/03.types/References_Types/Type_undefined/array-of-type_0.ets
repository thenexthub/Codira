/* * Copyright (c)  2023-2025 Huawei Device Co., Ltd.
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
desc: 3.8.9 The only value of type undefined is represented by the keyword undefined
name: array-of-type_0.ets
tags:

---*/

class Test {
    n : number | undefined = 1;
}

function check(b : boolean): int {
    return b ? 0 : 1;
}

function callMe<T>(a : Array<T>): int {
    return check(a[0] !== undefined)
}

function main(): int {
    let result: int = 0;
    result += callMe([{n: 42}, undefined]);
    return result;
}
