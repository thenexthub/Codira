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
desc: Equality operators, common rules
---*/

function main(): void {
    let a: int = 0;
    let b: int = 0;
    let c: int = 1;

    let cmp1: boolean = (a == b) == (b != c);
    let cmp2: boolean = (a == b) & b != c;

    arktest.assertEQ( cmp1, cmp2 )
}
