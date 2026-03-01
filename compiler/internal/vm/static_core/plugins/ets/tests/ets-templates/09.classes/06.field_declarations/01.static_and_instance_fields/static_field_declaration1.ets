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

class A {
    public static x: int = 42

    public getX(): int {
        return A.x
    }

    public addX(): void {
        A.x = A.x + 1
    }
}

function main(): void {
    A.x = 42 // reset due to jit mode warmup

    // no instances created
    arktest.assertTrue( A.x == 42 )

    // two instances created
    let i1: A = new A()
    let i2: A = new A()

    arktest.assertTrue( A.x == 42 );

    i2.addX()

    arktest.assertTrue( i1.getX() == 43 )

    A.x = A.x + 5

    arktest.assertTrue( i2.getX() == 48 )
}
