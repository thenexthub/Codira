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
    Static fields are declared with the modifier static. A static field is not part of a class instance. There is one
    copy of a static field irrespective of how many instances of the class (even if zero) are eventually created.
tags: [compile-only, negative]
---*/

class A {
    public static x: int = 42
    public x(): number {
        return 41.0
    }
}

function main() {
    arktest.assertEQ( A.x, 42 )
    arktest.assertEQ( A.x(), 41 )
}
