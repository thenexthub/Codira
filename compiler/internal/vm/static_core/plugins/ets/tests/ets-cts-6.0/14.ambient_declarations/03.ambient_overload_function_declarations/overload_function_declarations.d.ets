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
desc: Overload Ambient Function declarations
files: ['./overload_function_declarations.ets']
tags: [compile-only, not-a-test]
---*/

export declare function fun1(x: int): string
export declare function fun2(x: string): string
export declare overload fun {fun1, fun2}

export declare namespace N {
    function foo1(x: number, y: string, z: Object[]): double
    function foo2(x: number, y: string): double
    overload foo {foo1, foo2}
}

export declare function bar1<U, V, R>(x: U, y: V): R
export declare function bar2<U, R>(x: U): R
export declare overload bar {bar1, bar2}

export declare function baz1(y: string): string
export declare function baz2(y: string, x?: int): string
export declare overload baz {baz1, baz2}
