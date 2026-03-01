/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Void type in generic
---*/

class A<T> {
    f: T
    m(): T { return this.f }
    constructor(f: T) {this.f = f}
}

let a1: A<void> = new A<void>(undefined)
let a2: A<undefined> = new A<undefined>(undefined)

arktest.assertEQ(a1.f, undefined)
arktest.assertEQ(a2.f, undefined)
arktest.assertEQ(a2.m(), undefined)

function foo<T>(p: T): T { return p }

type F1<T> = () => T
const f1: F1<void> = (): void => {}
const f2: F1<void> = () => {}
const f3: F1<void> = (): undefined => { return undefined }

arktest.assertTrue(f1 instanceof F1)
arktest.assertTrue(f2 instanceof F1)
arktest.assertTrue(f3 instanceof F1)

type A1<T> = T[]
type A2<T> = Array<T>
const arr1: A1<void> = [undefined]
const arr2: A2<void> = [undefined, undefined]

arktest.assertEQ(arr1[0], undefined)
arktest.assertEQ(arr2[0], undefined)
