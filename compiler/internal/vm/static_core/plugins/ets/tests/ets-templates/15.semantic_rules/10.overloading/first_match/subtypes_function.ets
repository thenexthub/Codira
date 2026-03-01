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
desc: More "specific" methods are placed before more "general" candidates in the effective set 
---*/

/*
I1        I2 
 \       /
  C1    I3
    \  /
     C2
*/

interface I1 {}
class C1 implements I1 {}
interface I2 {}
interface I3 extends I2 {}
class C2 extends C1 implements I3 {}

function foo1(p: (p: C2) => void): string { return "foo1" }
function foo2(p: (p: C1) => void): string { return "foo2" }
function foo3(p: (p: I1) => void): string { return "foo3" }
function foo4(p: (p: I2) => void): string { return "foo4" }
function foo5(p: (p: I3) => void): string { return "foo5" }

// from less specific to more
overload foo {foo1, foo2, foo3, foo4, foo5}

// from more specific to less 
overload bar {foo5, foo4, foo3, foo2, foo1}

let c1 = new C1
let c2 = new C2
let i1: I1 = {}
let i2: I2 = {}
let i3: I3 = {}

// foo
arktest.assertEQ(foo((p: C2) => {}), "foo1")
arktest.assertEQ(foo((p: C1) => {}), "foo1")
arktest.assertEQ(foo((p: I1) => {}), "foo1")
arktest.assertEQ(foo((p: I2) => {}), "foo1")
arktest.assertEQ(foo((p: I3) => {}), "foo1")

// bar
arktest.assertEQ(bar((p: C2) => {}), "foo1")
arktest.assertEQ(bar((p: C1) => {}), "foo2")
arktest.assertEQ(bar((p: I1) => {}), "foo3")
arktest.assertEQ(bar((p: I2) => {}), "foo5")
arktest.assertEQ(bar((p: I3) => {}), "foo5")
