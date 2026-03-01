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
    Class level scope, public access
    A name declared inside a class (class level scope) is accessible (see Accessible) in the class and sometimes,
    depending on the access modifier (see Access Modifiers), outside the class, or by means of a derived class.
---*/

let x = -10

class C {
    public x: int = 1;
}

class D {
    private x: C;

    public constructor(p: C) {
        this.x = p;
    }
    public get():int {
        return this.x.x;
    }
    public put(p: int) {
        this.x.x = p;
    }
}

function main(): void {
    // access from D
    let d:D = new D(new C())
    arktest.assertEQ( d.get(), 1 )
    d.put(42)
    arktest.assertEQ( d.get(), 42 )

    // access from module
    let c:C = new C();
    arktest.assertEQ( c.x, 1 )
    c.x = 100
    arktest.assertEQ( c.x, 100 )

    // module level scope is not affected
    arktest.assertEQ( x, -10)
}
