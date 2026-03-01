/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
  tags: [not-a-test]
---*/

// deep inlining
export function C(x: int): int { return x + 1; }
export function B(x: int): int { return C(x); }
export function A(x: int): int { return B(x); }


// deopt
export function risky(arr: int[], i: int): int {
    return maybe_throw(arr[i]);
}

// deopt
export function maybe_throw(x: int): int {
    if (x == 0) {
        throw new Error('Error message');
    }
    return 100 / x;
}

// bounds check fails
export function sum_array(arr: int[]): int {
    let sum = 0;
    for (let i = 0; i <= arr.length; i++) {
        sum += arr[i];
    }
    return sum;
}
