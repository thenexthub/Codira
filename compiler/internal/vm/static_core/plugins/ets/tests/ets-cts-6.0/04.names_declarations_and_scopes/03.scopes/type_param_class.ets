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
  The scope of a type parameter name in a class or interface declaration is
  that entire declaration, excluding static member declarations.
---*/

let T: String = "str"

class C<T extends Number> {
    fld: T
    constructor(i: T) {
        this.fld = i
    }
    public foo(p: T): T {
        let i: T = p
        return i;
    }
}

function main(): void {
    let c: C<Number> = new C<Number>(1.0);

    arktest.assertEQ( c.foo(2.0), 2.0 )
    arktest.assertEQ( c.fld, 1.0 )

    // module level scope is not affected
    arktest.assertEQ( T, "str" )
}
