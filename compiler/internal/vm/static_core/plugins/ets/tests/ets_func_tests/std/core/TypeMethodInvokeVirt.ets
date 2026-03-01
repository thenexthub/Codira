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

abstract class A {
    abstract foo(): int;
}

class C extends A {
    override foo(): int {
        return 30
    }
}

function main(): void {
    const a = Type.resolve("TypeMethodInvokeVirt.A") as ClassType
    let got = 0
    for (let i = 0; i < a.getMethodsNum(); i++) {
        const m = a.getMethod(i)
        if (m.getName() != "foo") {
            continue
        }
        got++
        arktest.assertEQ( m.invoke(new C(), new FixedArray<Any>(0)) as Int, 30)
    }
    arktest.assertEQ( got, 1)
}
