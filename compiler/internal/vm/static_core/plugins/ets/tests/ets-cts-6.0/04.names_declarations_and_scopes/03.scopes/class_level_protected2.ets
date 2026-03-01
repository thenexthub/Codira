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
    Class level scope, mixed access
    A name declared inside a class (class level scope) is accessible (see Accessible) in the class and sometimes,
    depending on the access modifier (see Access Modifiers), outside the class, or by means of a derived class.
---*/

let x = -10

class C {
    private _x: int = 100;

    protected get x():int {
        return this._x;
    }

    protected set x(p: int) {
        this._x = p;
    }
}

class D extends C {
    private xx: int = 10;

    public get():int {
        return this.xx;
    }
    public put(p: int) {
        this.xx = p;
    }

    public getSuper():int {
        return super.x;
    }
    public putSuper(p: int) {
        super.x = p;
    }
}

function main(): void {
    let d: D = new D()

    arktest.assertEQ( d.get(), 10 )
    arktest.assertEQ( d.getSuper(), 100 )

    d.put(42)
    arktest.assertEQ( d.get(), 42 )
    arktest.assertEQ( d.getSuper(), 100 )

    d.putSuper(22)
    arktest.assertEQ( d.get(), 42 )
    arktest.assertEQ( d.getSuper(), 22 )

    // module level scope is not affected
    arktest.assertEQ( x, -10 )
}
