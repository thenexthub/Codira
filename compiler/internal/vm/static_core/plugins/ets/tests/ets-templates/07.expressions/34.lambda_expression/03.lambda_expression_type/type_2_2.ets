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
desc: Return type can be inferred from the lambda body
---*/

function sum(a:int, b: int) {
    return a + b
}

function main() {
    let FO = (a:int, b: int) => () => sum(a, b)

    let F1 = FO(40, 2)
    let R = F1()
    arktest.assertTrue(F1 instanceof Function0)
    arktest.assertEQ(42, R)
}
