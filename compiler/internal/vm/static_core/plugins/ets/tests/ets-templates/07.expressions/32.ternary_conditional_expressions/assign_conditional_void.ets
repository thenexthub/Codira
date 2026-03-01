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
desc: Type of the conditional expression is determined as the union of types of the second and the third expressions
tags: [compile-only, negative]
---*/


function foo(): void {
}

final class C {
    static foo(): void {

    }
}

type BRF = (input: boolean ) => boolean
const bo: BRF = (input: boolean) => { return input }

function main(): void {
  let x = bo(true) ? foo() : C.foo()

}
