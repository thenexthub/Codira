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

const strings = ["123", " \tabc\n\v ", "\"'abc'\"", "Русские Буквы", "函数式编程"]
const validIndices: int[] = [0, 1]
const invalidIndices: int[] = [-1, -100, 200]

function getError(index: int): RangeError {
    return new RangeError(`Invalid index: ${index}`)
}

function getError(index: number): RangeError {
    return getError(index.toInt())
}

function emptyStr(): void {
    const str = ""
    for (const index of validIndices) {
        arktest.expectError(() => { str.charCodeAt(index) }, getError(index))
    }
    for (const index of invalidIndices) {
        arktest.expectError(() => { str.charCodeAt(index) }, getError(index))
    }
}

function singleChar(): void {
    const str = "𠮷"
    arktest.expectNoThrow(() => { str.charCodeAt(0) })
    for (const index of invalidIndices) {
        arktest.expectError(() => { str.charCodeAt(index) }, getError(index))
    }
}

function multipleChars(): void {
    for (const str of strings) {
        for (const index of validIndices) {
            arktest.expectNoThrow(() => { str.charCodeAt(index) })
        }
        for (const index of invalidIndices) {
            arktest.expectError(() => { str.charCodeAt(index) }, getError(index))
        }
    }
}

function main(): int {
    const suite = new arktest.ArkTestsuite("std.core.String.charCodeAt method tests")

    suite.addTest("empty string", emptyStr)
    suite.addTest("single character string", singleChar)
    suite.addTest("multiple characters strings", multipleChars)

    return suite.run()
}
