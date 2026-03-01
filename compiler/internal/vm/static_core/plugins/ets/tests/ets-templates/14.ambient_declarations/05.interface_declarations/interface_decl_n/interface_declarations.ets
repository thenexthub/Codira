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
---*/
  

export interface A { a: number }

export interface B {
    a: int
    b: float
}

export interface C {
    foo(x: number, y: string): string
}

export interface D<T1> {
    a: T1
}

export interface E<U, R> {
    foo(x: U): R
}

export interface F {
    f: string
}

export interface M extends F {
}

export interface K {
    a: string
    get foo(): string
    set foo(v: string)
}

export interface J {}