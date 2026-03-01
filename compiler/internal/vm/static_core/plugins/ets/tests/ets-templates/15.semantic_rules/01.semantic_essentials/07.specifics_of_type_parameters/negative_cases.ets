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
desc: Explicit type argument new Neg<number>(1) must be used to fix the code.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0170: Type 'Neg<T>' is generic but type argument were not provided."
---*/

class Neg<T extends number> {
    constructor (x: T) {}
}

function main() {
    new Neg(1)  // CTE: int is not a subtype of number
}
