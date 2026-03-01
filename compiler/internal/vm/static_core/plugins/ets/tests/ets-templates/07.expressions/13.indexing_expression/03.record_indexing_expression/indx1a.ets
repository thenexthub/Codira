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
desc: >-
    The type of the object reference expression must be a nullish type based on
    an array type, or on the Record type; a compile-time error occurs otherwise.
---*/

function refNotNullish(): Record<string, number> {
    let res: Record<string, number> = { "magic": 42 }
    return res
}

function main() {
    arktest.assertTrue(refNotNullish()["magic"] == 42)
    arktest.assertTrue(refNotNullish()?.["magic"] == 42)
}
