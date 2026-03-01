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
    The execution of a binary operator that is an integer division ‘/’,
    or integer remainder ‘%’
    can throw ArithmeticError only after the evaluations of both operands complete normally.
---*/

function main() {

    let a = new int[2];
    let x: int = 42;
    let exceptions: int = 0;

    try {
        x = a[2] / 0;  // eval should abort on left-hand
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError);
        exceptions++;
    }
    arktest.assertEQ(exceptions, 1);
    arktest.assertEQ(x, 42);

    try {
        x = a[2] % 0;  // eval should abort on left-hand
    } catch (e) {
        arktest.assertTrue(e instanceof RangeError);
        exceptions++;
    }
    arktest.assertEQ(exceptions, 2);
    arktest.assertEQ(x, 42);

    try {
        x = a[0] % a[1];  // both hands should be evaluated
    } catch (e) {
        arktest.assertTrue(e instanceof ArithmeticError);
        exceptions++;
    }
    arktest.assertEQ(exceptions, 3);
    arktest.assertEQ(x, 42);

    
}
