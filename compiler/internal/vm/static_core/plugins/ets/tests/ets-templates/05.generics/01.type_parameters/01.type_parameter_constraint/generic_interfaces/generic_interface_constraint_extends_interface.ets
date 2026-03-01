/*---
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
---*/

/*---
desc: >-
    A type parameter of a generic can depend on another type parameter of the same generic.
    If S constrains T, then the type parameter T directly depends on the type parameter S,
    while T directly depends on the following
      • S; or
      • Type parameter U that depends on S.
---*/

interface A {}
interface B extends A{}
interface C {}
interface D<T extends A> {}
interface E<T extends A | C> {}
interface F<T extends Object> {
    m(): T
    p: T
}

function main(): void {
    let i1: D<A> = {}
    let i2: D<B> = {}
    let i3: E<A> = {}
    let i4: E<B> = {}
    let i5: E<C> = {}
}

