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
desc: Shift expressions with enumerate values
---*/

enum Enm {A = -4, B = 1, C = -8, D = -2, E = 2147483646}

function main(): void {
    
    let left_shift = Enm.A << Enm.B
    arktest.assertEQ(left_shift, Enm.C)

    let signed_right_shift = Enm.A >> Enm.B
    arktest.assertEQ(signed_right_shift, Enm.D)

    let unsigned_right_shift = Enm.A >>> Enm.B
    arktest.assertEQ(unsigned_right_shift, Enm.E)
}
