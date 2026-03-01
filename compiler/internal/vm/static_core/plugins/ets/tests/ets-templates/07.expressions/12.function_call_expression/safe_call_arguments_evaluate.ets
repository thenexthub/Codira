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
desc: Arguments are not evaluated.
assert: if ’?.’ is provided and expression is evaluated as null then arguments are not evaluated and the call is not performed and result of the functionCallExpression is null too.
---*/

let ecf0 = 0;
function eval0(): String {
    ecf0++
    return 'Hello World'
}

let ecf1 = 0;
function eval1(): int {
    ecf1++
    return 100
}

let ecf2 = 0;
function eval2(): Boolean {
    ecf2++
    return true
}

type NullOrChaining = Chaining | null

class Chaining {
    static callc: int = 0;

    stepOne(rr: Boolean = false): NullOrChaining {
        return rr ? this : null
    }

    stepTwo(p1: String, p2: int, p3: Boolean) {
        Chaining.callc++
    }
}

function main(): void {
    ecf0 = 0;
    ecf1 = 0;
    ecf2 = 0;
    Chaining.callc = 0
    new Chaining().stepOne()?.stepTwo(eval0(), eval1(), eval2())
    arktest.assertEQ(0, ecf0)
    arktest.assertEQ(0, ecf1)
    arktest.assertEQ(0, ecf1)
    arktest.assertEQ(0, Chaining.callc)

    ecf0 = 0;
    ecf1 = 0;
    ecf2 = 0;
    Chaining.callc = 0
    new Chaining().stepOne(true)?.stepTwo(eval0(), eval1(), eval2())
    arktest.assertEQ(1, ecf0)
    arktest.assertEQ(1, ecf1)
    arktest.assertEQ(1, ecf1)
    arktest.assertEQ(1, Chaining.callc)

}
