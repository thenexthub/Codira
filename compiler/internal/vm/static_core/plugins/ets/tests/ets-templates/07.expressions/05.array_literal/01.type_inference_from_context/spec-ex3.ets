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
  The type of an array literal can be inferred from the context,
  including explicit type annotation of a variable declaration,
  left-hand part type of an assignment, call parameter type,
  or type of a cast expression.
tags: [compile-only, negative]
---*/

function main() {
    let incorrect: [number, string] = ["hello", 1] // compile-time error
}
