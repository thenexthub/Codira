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

// static call without any side effects
export function add(x: int, y: int): int {
    return x + y;
}

// functions with unpredictable side effects
export function add_with_log(x: int, y: int): int {
    console.log('Adding..');
    return x + y;
}

export function rand_plus(x: int): double {
    let r = Math.floor(Math.random() * 10);  // [0, 9]
    return x + r;
}

export function get_arr_element(arr: int[], idx: int): int {
    return arr[idx];
}

export function make_array(n: int): int[] {
    let arr = new int[n];
    return arr;
}

export function stringify(x: Object): string {
    return x.toString();
}

export function safe_div(x: int, y: int): int {
    return x / y;
}
