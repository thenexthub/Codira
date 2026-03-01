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

export native function sum(a: int, b: int): int;
export native function sum(a: int, b: int, c: int): int;
export native function concat(s0: String, s1: String, s2: String): String;
export native function concat(s0: String, s1: String): String;
export native function recursiveFunction(value: int): int;
export native function checkSignature(a: String): void;

function checkSum(): boolean {
    return sum(8, 16) == 24;
}

function checkSumA(): boolean {
    return sum(8, 16, 6) == 30;
}

function checkConcatA(): boolean {
    return concat("abc", "def", "ghi") == "abcdefghi";
}

function checkConcat(): boolean {
    return concat("abc", "def") == "abcdef";
}

namespace test001A {
    class TestA001 {
        constructor() {
        }
        public native foo(): int;
        public native long_foo(): long;
    }
}