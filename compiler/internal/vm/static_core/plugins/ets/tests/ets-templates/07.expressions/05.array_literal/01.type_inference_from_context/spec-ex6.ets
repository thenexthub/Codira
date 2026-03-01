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
  If the type used in the context is a union type (see Union Types),
  then it is necessary to try inferring the type of the array literal
  from its elements (see Array Type Inference from Types of Elements).
  If successful, then the type so inferred must be compatible with one
  of the types that form the union type.
---*/

function main() {
    let union_of_arrays: number[] | string[] = [1.0, 2.0]  // OK, type of literal is number[]

    arktest.assertEQ((union_of_arrays as number[])[0], 1.0)
}
