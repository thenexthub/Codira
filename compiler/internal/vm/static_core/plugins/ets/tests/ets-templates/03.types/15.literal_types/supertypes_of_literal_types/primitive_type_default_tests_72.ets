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
desc: Type compatibility test for boolean type in logical OR operation condition checking.
---*/

function orConditionTest() {
    let bool1 = false;
    let bool2 = true;
    let result = bool1 || bool2;
    return result;
}

function main(): void {
    const finalResult = orConditionTest();
    arktest.assertEQ(finalResult, true);
}
