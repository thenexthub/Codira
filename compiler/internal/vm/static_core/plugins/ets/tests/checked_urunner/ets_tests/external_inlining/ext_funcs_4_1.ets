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

import { risky_array } from './ext_funcs_4.ets';

export function middle(arr: int[]): int {
    return risky_array(arr); // inline will be here
}

export function compute(x: int): int {
    return x * x + 1;
}

// potential deoptimization
export function compute2(x: int): int {
    if (false) { // SaveState will be created
        throw new Error('Unreachable!');
    }
    return x * x + 1;
}
