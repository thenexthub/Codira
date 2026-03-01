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
desc: Test union type normalization. If at least one type in a union is Any, then all other types are removed.
---*/

type A = Any | string;

class TestClass {
    test: null
}

function main(): void {
    let anyVar: A
    anyVar = 1
    arktest.assertEQ(typeof anyVar, "int")
    anyVar = "test" 
    arktest.assertEQ(typeof anyVar, "string")
    anyVar = undefined
    arktest.assertEQ(typeof anyVar, "undefined")
    anyVar = () => {}
    arktest.assertEQ(typeof anyVar, "function")
    anyVar = new TestClass()
    arktest.assertEQ(typeof anyVar, "object")
    anyVar = c'◯'
    arktest.assertEQ(typeof anyVar, "char")
}
