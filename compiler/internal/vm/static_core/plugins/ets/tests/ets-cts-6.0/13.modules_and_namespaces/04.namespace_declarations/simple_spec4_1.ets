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
desc: >-
    Sample from spec, Namespaces with identical namespace names in a single compilation unit
    merge their exported declarations into a single namespace.
---*/

namespace A {
    export function foo(): string {return "A foo"}
    function bar(): string {return "A bar"}

    export namespace C {
        export function too(): string {return "A too"}
    }
}

namespace A {
    export function goo(): string {
        arktest.assertEQ(A.foo(), "A foo") // calls exported foo()
        arktest.assertEQ(foo(), "A foo") /* calls exported foo() as well as all A namespace
                                        declarations are merged into one */
        arktest.assertEQ(A.C.moo(), "A too")
        return bar() // bar() belongs to the same namespace
    }
}

namespace A.C {
    export function moo() {
        arktest.assertEQ(too(), "A too") // too()  accessible when namespace C and too() are both exported
        return A.C.too()
    }
}

namespace B { }

function main() {
    arktest.assertEQ(A.goo(), "A bar")
}
