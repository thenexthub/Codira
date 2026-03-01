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
desc: Test joint type.
---*/

type T1 = string;
type T2 = number;
type T3 = boolean;
type T4 = null;
type Union = (T1 | T2) | (T3 | T4);

function main(): void {

    let a: Union = "test";
    arktest.assertEQ(typeof a, "string");
    let b: Union = 10;
    arktest.assertEQ(typeof b, "number");
    let c: Union = true;
    arktest.assertEQ(typeof c, "boolean");
    let d: Union = null;
    arktest.assertEQ(typeof d, "object");
}
