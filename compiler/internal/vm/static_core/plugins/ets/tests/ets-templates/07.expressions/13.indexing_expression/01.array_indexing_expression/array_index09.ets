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
desc: Test the array index expression to Out-of-bounds access of multi-dimensional arrays.
---*/

function main(): void {
    let matrix: number[][] = [
      [1, 2, 3],
      [4, 5, 6],
      [7, 8, 9]
    ];

    let isError: boolean = false;

    try {
        matrix[3][0];
    } catch (error) {
        error = error as Error;
        arktest.assertEQ('RangeError', error.name);
        arktest.assertEQ('Out of bounds', error.message);
        isError = true;
    } finally {
        if (!isError) {
            throw new Error('No RangeError occured, but "Out of bounds" expected.');
        }
    }
}
