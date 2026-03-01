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
desc: First simple sample from spec
tags: [not-a-test, compile-only, negative]
---*/

export namespace A {
    function foo(): void {}
    function bar(): void {}

    export namespace C {
        function too(): void {}
    }
}

export namespace A {
    function goo(): void { bar() }    // bar() belongs to the same namespace
//    function foo(): void        // Compile-time error as foo() was already defined
}

export namespace class {
    export function foo() {
        return "foo";
    }
}

export namespace abstract {
    export function foo() {
        return "foo";
    }
}
