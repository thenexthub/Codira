/**
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



namespace ops {
export native function sum(a: int, b: int): int;
export native function sum(a: int, b: int, c: int): int;
export native function concat(s0: String, s1: String): String;
}

function checkSum(): boolean {
    return ops.sum(8, 16) == 24;
}

function checkSumA(): boolean {
    return ops.sum(8, 16, 6) == 30;
}

function checkConcat(): boolean {
    return ops.concat("abc", "def") == "abcdef";
}

namespace A {
    export native function recursiveFunction(value: int): int;
    namespace B {
        export native function sumB(a: int, b: int): int;
    }
    function checkSumB(): boolean {
        return B.sumB(8, 16) == 24;
    }
}
function checkRecursive(): boolean {
    return A.recursiveFunction(5) == 15;
}