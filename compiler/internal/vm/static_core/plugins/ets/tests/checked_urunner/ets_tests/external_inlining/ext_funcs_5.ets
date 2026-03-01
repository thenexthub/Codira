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

import { compute } from './ext_funcs_4_1.ets';
import { compute2 } from './ext_funcs_4_1.ets';

export function top(x: int): int {
    let acc = 0;
    for (let i = 0; i < x; i++) {
        acc += compute(i);  // inline here
    }
    return acc;
}

export function top2(x: int): int {
    let acc = 0;
    for (let i = 0; i < x; i++) {
        acc += compute2(i);
    }
    return acc;
}
