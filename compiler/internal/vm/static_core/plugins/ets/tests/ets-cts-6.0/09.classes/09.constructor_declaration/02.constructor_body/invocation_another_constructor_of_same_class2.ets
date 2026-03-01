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
desc: Invocation constructor of same class.
assert: >-
    The first statement of a constructor body may be an explicit invocation of another constructor of the same class
    or of the direct superclass.
---*/

class A {
    public x: int;

    public constructor() {
        this(5);
    }

    public constructor(a: int) {
        this.x = a;
    }
}

function main() {
    arktest.assertEQ(new A().x, 5)
    arktest.assertEQ(new A(1).x, 1)
}
