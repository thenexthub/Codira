/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    Superclass constructor calls (used to call a direct superclass constructor),
    called Unqualified superclass constructor calls that start with the keyword super
---*/

class A {
    x: int = 0;

    constructor() {
        this(5)
        this.x++
    }
    constructor(a: int) {
        this(true)
        this.x++
    }
    constructor(a: boolean) {
        this('abc')
        this.x++
    }
    constructor(a: string) {
        this(1.0f)
        this.x++
    }
    constructor(a: float) {
        this(new Error)
        this.x++
    }
    constructor(a: Error) {
        this([1, 2, 3])
        this.x++
    }
    constructor(a: Array<int>) {
        this(() => {})
        this.x++
    }
    constructor(a: () => void) {
        this.x++
    }
}

class B extends A {
    constructor() {
        super(5)
        this.x++
    }
    constructor(a: int) {
        super(true)
        this.x++
    }
    constructor(a: boolean) {
        super('abc')
        this.x++
    }
    constructor(a: string) {
        super(1.0f)
        this.x++
    }
    constructor(a: float) {
        super(new Error)
        this.x++
    }
    constructor(a: Error) {
        super([1, 2, 3])
        this.x++
    }
    constructor(a: Array<int>) {
        super(() => {})
        this.x++
    }
    constructor(a: () => void) {
        this.x++
    }
}

function main(): void {
  let i1: B = new B();
  arktest.assertEQ(i1.x, 8);

  i1 = new B(1.0f);
  arktest.assertEQ(i1.x, 4);
}
