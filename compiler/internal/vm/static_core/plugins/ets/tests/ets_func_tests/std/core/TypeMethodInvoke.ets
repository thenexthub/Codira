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

class C {
    getIsEvenPlusX(x: int): boolean {
        return (this.a + x) % 2 == 0
    }

    static sum(a: int, b: int): long {
        return a.toLong() + b
    }

    a: int = 0
}

function testSum(sum: Method): void {
    const args : FixedArray<Any> = new FixedArray<Any>(2)
    args[0] = Int.MAX_VALUE
    args[1] = 30
    const res = sum.invoke(null, args)
    arktest.assertEQ( res, (Int.MAX_VALUE.toLong() + 30))
}

function testGetIsEvenPlusX(getIsEvenPlusX: Method): void {
    const c = new C()
    c.a = 15
    const args : FixedArray<Any> = new FixedArray<Any>(1)

    args[0] = 13
    arktest.assertEQ( getIsEvenPlusX.invoke(c, args), true)

    c.a = 14
    arktest.assertEQ( getIsEvenPlusX.invoke(c, args), false)

    args[0] = 30
    arktest.assertEQ( getIsEvenPlusX.invoke(c, args), true)
}

function main(): void {
    const c = new C()
    const cType = Type.of(c) as ClassType
    const meths = cType.getMethodsNum()
    let got = 0
    for (let i = 0; i < meths; i++) {
        const m = cType.getMethod(i)
        if (m.getName() == "sum") {
            testSum(m)
            got++
        }
        if (m.getName() == "getIsEvenPlusX") {
            testGetIsEvenPlusX(m)
            got++
        }
    }
    arktest.assertEQ( got, 2)
}
