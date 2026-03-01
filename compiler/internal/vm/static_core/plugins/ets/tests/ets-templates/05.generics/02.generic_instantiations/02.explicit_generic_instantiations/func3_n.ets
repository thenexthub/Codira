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
        An explicit generic instantiation is a language construct, which provides a list of type arguments
        (see Type Arguments) that specify real types or type parameters to substitute corresponding type parameters of a generic.
tags: [compile-only, negative]
expected_out: "Semantic error ESE655063: Generic function foo requires explicit type arguments when used as a value."
---*/

// function arguments with tuple
function foo<T, U>(p: T, q: U): [T, U] {
    return [p, q]
}

function main() {
    let v = foo<function, function>((p: number): number => p * p, (p: number): number => p + p)
    arktest.assertTrue(v[0](5.0) == 25.0 && v[1](42.0) == 84.0)
    arktest.assertTrue(v[0](5.0) instanceof number && v[1](42.0) instanceof number)
}
