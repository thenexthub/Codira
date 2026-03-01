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
    Any right-hand expression is evaluated only after
    the left-hand expression of a binary operator is fully evaluated.
---*/

function main() {

    let a = new int[2];
    let x: int = 1;
    let y: int = 1;
    let exceptions: int = 0;

    try {
        y = (a[3]) * (--x);  // eval should abort on left-hand
    } catch (e) {
        arktest.assertEQ(x,1)
        arktest.assertEQ(y,1)
        exceptions++;
    }
    arktest.assertEQ(exceptions, 1);

    try {
        y = (x++) + (a[3]);  // eval should abort on right-hand
    } catch (e) {
        arktest.assertEQ(x,2)
        arktest.assertEQ(y,1)
        exceptions++;
    }
    arktest.assertEQ(exceptions, 2);

}
