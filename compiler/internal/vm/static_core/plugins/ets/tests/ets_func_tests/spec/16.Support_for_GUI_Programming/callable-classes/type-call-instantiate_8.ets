/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: 16.6 Callable Types
name: type-call-instantiate_8.ets
tags: []


---*/


class CallMeTrinity {
    value: int = 0;

    static $_instantiate(factory: () => CallMeTrinity, ...values: int[]): CallMeTrinity {
        let x = factory();
        x.value = values.length
        return x;
    }

    getValue(): int {
        return this.value;
    }

}

function main(): int {
    let tt: CallMeTrinity = CallMeTrinity(1, 2, 3, 4);
    console.log("Value: " + tt.getValue())
    return tt.getValue() == 4 ? 0 : 1;
}
