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
 desc: A function reference refers to a declared or imported function.
       A compile-time error occurs if an overload alias is used in a named reference

 ---*/

function fooE(): void {}
function fooS(x: string): string { return x }

overload foo { fooE, fooS }

const fooRef = fooS

class TestFR {
    cfoo = fooRef

    call(): string {
        return this.cfoo('Test')
    }

    static fr = fooRef
}

function main() {
    let fr0: Function = fooE
    let fr1 = (() => fooS )()
    fr0()
    arktest.assertEQ('Test', fr1('Test'))

    let tc = new TestFR
    arktest.assertEQ('Test', tc.call())
    arktest.assertEQ('Test', TestFR.fr('Test'))

}
