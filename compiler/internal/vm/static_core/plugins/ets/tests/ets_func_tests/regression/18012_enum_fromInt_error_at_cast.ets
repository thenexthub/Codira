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
 desc: Issue 18012. Cannot find Enum fromInt method when cast to int
 tags: []
 ---*/

enum Color {Red, Green, Blue, Black = -1}

function castIntToString(val: int): string {
    let cr = "" + ((Color.fromValue(val)))
    return cr
}

function handleBadValue(val: int): int {
    try {
        castIntToString(val)
    } catch (e) {
        return 0;
    }
    return 1;
}

function main(): int {
    let cr = castIntToString(1)
    arktest.assertEQ<String, String>(cr,"1");
    return handleBadValue(4)
}
