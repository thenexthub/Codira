/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    A static field is instantiated when the class is initialized. A static field
    can have only one instantiation, irrespective of how many instances of that
    class (even if zero) are eventually created.
---*/
type Func = (p: number) => number

class A {
    private static f: Func = (p: number): number => { return 0 }

    public getF(): Func {
        return A.f
    }

    public setF(p: Func) {
        A.f = p
    }
}

function main(): void {
    let a1 = new A()
    a1.setF((p: number): number => { return p + 33 })
    let f1: Func = a1.getF()
    arktest.assertEQ( f1(11), 44 )

    let a2 = new A()
    let f2: Func = a2.getF()
    arktest.assertEQ( f2(22), 55 )

    arktest.assertTrue( f1 === f2 )

    a1.setF((p: number): number => { return p + 1 })
    f1 = a1.getF()
    arktest.assertEQ( f1(11), 12 )

    f2 = a2.getF()
    arktest.assertEQ( f2(22), 23 )

    arktest.assertTrue( f1 === f2 )
}
