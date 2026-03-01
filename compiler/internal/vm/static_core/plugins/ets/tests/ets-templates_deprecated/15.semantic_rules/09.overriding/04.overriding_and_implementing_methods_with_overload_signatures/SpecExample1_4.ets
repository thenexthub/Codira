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
desc: Spec example
tags: []

---*/

interface Interface {
    foo(p: number): void
    foo(p: string): void
}

class Class1 implements Interface {
    foo(p: number): void
    foo(p: string): void
    foo(...p: Any[]): Any {}
}

class Class3 extends Class1 {
    override foo(p: number): void// 1st overload signature
    override foo (p: string): void // 2nd overload signature
    override foo (...p: Any[]): Any {} // implementation signature + body
}

function test(value: Interface) {
    value.foo(5)
    value.foo("5555")
}

function main() {

    test(new Class3)
}
