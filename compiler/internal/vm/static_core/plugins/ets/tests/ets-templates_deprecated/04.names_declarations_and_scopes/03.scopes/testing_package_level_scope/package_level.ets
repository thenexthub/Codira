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
desc: Package level scope, package module
package: testing_package_level_scope
---*/

package testing_package_level_scope;

let x = 1;

function foo(): int {
    return x - 1;
}

class C {
    y: int = x + 1;
}

interface I {
    z(): int {
        return x * 3;
    }
}

class D implements I {}

function main(): void {
    arktest.assertEQ( x, 1 )
    arktest.assertEQ( new C().y, 2 )
    arktest.assertEQ( new D().z(), 3 )
    arktest.assertEQ( foo(), 0 )
}
