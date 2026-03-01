/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Invocation constructor of superclass.
assert: >-
    The first statement of a constructor body may be an explicit invocation of another constructor of the same class
    or of the direct superclass.
---*/

class A {
    public x: int

    constructor(a: int) {
        this.x = a
    }
}

class B extends A {
    constructor() {  // secondary
        this(5)
    }

    constructor(p: int) {  // primary
        super(p)
    }
}

function main() {
    arktest.assertEQ(new A(42).x, 42)
    arktest.assertEQ(new B(33).x, 33)
    arktest.assertEQ(new B().x, 5)

    let b: B = {}
    arktest.assertEQ(b.x, 5)

    b = { x: -1 }
    arktest.assertEQ(b.x, -1)
}
