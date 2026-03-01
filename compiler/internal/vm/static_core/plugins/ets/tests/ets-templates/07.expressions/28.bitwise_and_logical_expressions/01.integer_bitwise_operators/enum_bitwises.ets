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
desc: Bitwise expressions with enumerate values
---*/

enum Enm {A = 1, B = 4, C = 3, D = 6} 

function main(): void {
    
    let and_1 = Enm.A & Enm.B
    let and_2 = Enm.C & Enm.D
    arktest.assertEQ(and_1, 0)
    arktest.assertEQ(and_2, 2)

    let exc_or_1 = Enm.A ^ Enm.B
    let exc_or_2 = Enm.C ^ Enm.D
    arktest.assertEQ(exc_or_1, 5)
    arktest.assertEQ(exc_or_2, 5)

    let inc_or_1 = Enm.A | Enm.B
    let inc_or_2 = Enm.C | Enm.D
    arktest.assertEQ(inc_or_1, 5)
    arktest.assertEQ(inc_or_2, 7)

}
