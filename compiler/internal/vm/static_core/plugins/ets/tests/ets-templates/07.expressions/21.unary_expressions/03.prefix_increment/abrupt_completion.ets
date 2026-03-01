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
    Otherwise, the prefix increment expression completes abruptly,
    and no incrementation occurs.
---*/

class A {
    private _i: int = 42;
    get i(): int {
        if (this._i == 13) {
            throw new Error()
        }
        return this._i
    }
    set i(v: int) { this._i = v }
    get_i(): int { // bypass getter
        return this._i;
    }
}


function main(): void {
    let a = new A
    let check: int = -100
    a.i = 13
    try {
        check = ++a.i
    } catch (e) {
        arktest.assertEQ(a.get_i(), 13)
    }
    arktest.assertEQ(a.get_i(), 13)
    arktest.assertEQ(check, -100)
}
