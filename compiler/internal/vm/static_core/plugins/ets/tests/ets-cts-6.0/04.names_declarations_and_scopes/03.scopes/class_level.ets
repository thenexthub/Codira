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
desc: Class level scope
---*/

let x = 3

function foo(): int {
    return x;
}

class C {
    x: int = 1;
    foo(): int {
        return this.x;
    }
}

class D {
    x: int = 2;
    foo(): int {
        return this.x;
    }
}

function main(): void {
    let x = 4;
    arktest.assertEQ( new C().foo(), 1 )
    arktest.assertEQ( new D().foo(), 2 )
    arktest.assertEQ( foo(), 3 )
    arktest.assertEQ( x, 4 )
}
