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

class A {
    public static x: int
}

class B extends A {
    public static y: int

    public static getX(): int {
        return A.x
    }

    public static setX(value: int): void {
        A.x = value
    }
}

function main(): void {
    B.y = 10
    B.setX(5)

    arktest.assertEQ( A.x, 5 )

    arktest.assertEQ( B.getX(), 5 )
    arktest.assertEQ( B.y, 10 )

    B.y++
    A.x++

    arktest.assertEQ( A.x, 6 )

    arktest.assertEQ( B.getX(), 6 )
    arktest.assertEQ( B.y, 11 )
}
