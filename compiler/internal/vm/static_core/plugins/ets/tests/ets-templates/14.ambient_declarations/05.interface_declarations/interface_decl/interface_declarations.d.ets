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
desc: Ambient Interface declarations
tags: [compile-only, not-a-test]
files: ['./interface_declarations.ets']
---*/
  

export declare interface A { a: string }

export declare interface B {
    a: int
    b: float
    c: Byte[]
}

export declare interface C {
    foo(x: number, y: string, z: double[]): string
}

export declare interface D<T1, T2> {
    a: T1
    b: T2
}

export declare interface E<U, V, R> {
    foo(x: U, y: V): R
}

export declare interface F {
}

export declare interface M extends F {
}

export declare interface K {
    get foo(): int
    set foo(v: int)
}

export declare interface J {
    default foo(f: number): string
}