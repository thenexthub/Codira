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
type AAlias = A
interface B extends AAlias {}
type BAlias = B
interface C {}
type CAlias = C
interface D<T extends AAlias> {}
type DAlias<T> = D<T>
interface E<T extends AAlias | CAlias> {}

function main(): void {
    let i1: DAlias<AAlias> = {}
    let i2: DAlias<BAlias> = {}
    let i3: E<AAlias> = {}
    let i4: E<BAlias> = {}
    let i5: E<CAlias> = {}
    arktest.assertTrue(i1 instanceof D)
    arktest.assertTrue(i2 instanceof D)
    arktest.assertTrue(i3 instanceof E)
    arktest.assertTrue(i4 instanceof E)
    arktest.assertTrue(i5 instanceof E)
}

