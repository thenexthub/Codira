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
desc: Type compatibility test for number type in array element summation.
---*/

function sumArrayElements(arr: number[]) {
    let total = 0;
    for (let num of arr) {
        total += num;
    }
    return total;
}

function main(): void {
    const input: number[] = [2, 4, 6];
    const result = sumArrayElements(input);
    arktest.assertEQ(result, 12);
}
