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
desc: >-
    If an interface has a method implementation, then its object literal can optionally provide
    a new method implementation. Otherwise, the interface implementation is used
---*/

interface I {
    method(): number { return 1 }
}

// Valid literal of anonymous class type using interface method
const i1: I = {}

// Valid literal of anonymous class type using own metod declaration
const i2: I = {
    method(): number { return 2 }
}

function main(): void {
    arktest.assertEQ(i1.method(), 1)
    arktest.assertEQ(i2.method(), 2)
}