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
desc: Invocation constructor of abstract superclass.
assert: >-
    The first statement of a constructor body may be an explicit invocation of another constructor of the same class
    or of the direct superclass.
---*/

abstract class A {
    x: int;

    constructor fromInt(a: int) {
        this.x = a + 1
    }
}

abstract class B extends A {
    constructor(a: int) {  // secondary constructor
        this.fromInt(a + 1)
    }

    constructor fromInt(a: int) { // primary constructor
        super.fromInt(a + 1)
    }
}

class C extends B {
    constructor() {  // secondary constructor
        this.fromInt(42)
    }

    constructor fromInt(a: int) { // primary constructor
        super(a + 1)
    }
}


function main() {
    let c: C = {}
    arktest.assertEQ(c.x, 46)

    c = new C.fromInt(1)
    arktest.assertEQ(c.x, 5)
}
