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
  The scope of a type parameter name in a function declaration is that entire declaration (function parameter scope).
---*/

let T: Number = 1.0

function foo<T extends Number>(p: T): T {
    let x: T
    {
        let c: T = p
        x = c
    }
    return x
}

function main(): void {
    arktest.assertEQ( foo<Number>(new Number(2.0)), 2.0 )

    // module level scope is not affected
    arktest.assertTrue( T == 1.0 )
}
