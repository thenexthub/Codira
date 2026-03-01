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
    A static field is instantiated when the class is initialized. A static field can have
    only one instantiation, irrespective of how many instances of that class (even if zero)
    are eventually created.
---*/

class B {}

let theInstance: B = new B()

class A {
    public static b1: B = theInstance // static field
    public b2: B = new B() // instance field
}

function main(): void {
    arktest.assertTrue( A.b1 === theInstance )

    // two instances created
    let i1: A = new A()
    let i2: A = new A()

    arktest.assertTrue( A.b1 === theInstance )
    arktest.assertTrue( i1.b2 !== theInstance )
    arktest.assertTrue( i2.b2 !== theInstance )
}
