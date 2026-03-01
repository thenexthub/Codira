/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Issue 30856. Segfault on type alias
tags: [compile-only, negative]
expected_out: ["Syntax error ESY0179: Required type parameters may not follow optional type parameters."]
---*/

type X<T extends R, R extends T | undefined = T, U extends T> = U<T<R>>

function main() {}