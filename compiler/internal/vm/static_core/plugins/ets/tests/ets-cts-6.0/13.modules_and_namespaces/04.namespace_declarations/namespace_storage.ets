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
desc: Different Namespaces in common file
tags: [not-a-test, compile-only]
---*/

namespace NS1 {
    export function sayHello() {
        return "Hello from NS1";
    }
    export let nsVar = 1234
    export const nsConst = 1234
}

namespace A.B.C {
    export function foo() {
        return "foo";
    }
}

export namespace Space1 {
    export function foo() {
        return "foo";
    }
    export let nsVar = 1234
    export const nsConst = 2345
}

export namespace Space2 {
    export function foo(p: number): number {
        return p;
    }
    export let nsVar = "1234"
}
export namespace NS2 {
    export function sayHello() {
        return "Hello from NS2";
    }
    export let nsVar = 1234
    export const nsConst = 1234
}

export namespace Transit {
    export function foo(): string {return "Transit foo"}
    function bar(): string {return "Transit bar"}

    export namespace C {
        export function too(): string {return "Transit too"}
    }
}
