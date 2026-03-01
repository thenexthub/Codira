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
desc: comparison with utility types and local smart types
name: comparison_utulity_types
---*/

function main(): void {
    let one: NonNullable<int> = 1
    let two: Readonly<int> = 2
    let three: Required<Readonly<number>> = 3.1
    let four: Required<NonNullable<Readonly<number>>> = 4.0
    //local smart type, effectively number
    let five: number|string = 5

    arktest.assertEQ(one > 0, true)
    arktest.assertEQ(two <= 3, true)
    arktest.assertEQ(three >= 3, true)
    arktest.assertEQ(four < 5, true)
    arktest.assertEQ(five <= 5, true)
}
