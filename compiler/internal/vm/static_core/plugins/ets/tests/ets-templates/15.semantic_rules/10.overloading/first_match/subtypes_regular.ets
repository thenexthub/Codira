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

// from less specific to more
function foo(p: C2): string { return "foo1" }
function foo(p: C1): string { return "foo2" }
function foo(p: I1): string { return "foo3" }
function foo(p: I2): string { return "foo4" }
function foo(p: I3): string { return "foo5" }

// from more specific to less
function bar(p: I3): string { return "bar5" }
function bar(p: I2): string { return "bar4" }
function bar(p: I1): string { return "bar3" }
function bar(p: C1): string { return "bar2" }
function bar(p: C2): string { return "bar1" }

let c1 = new C1
let c2 = new C2
let i1: I1 = {}
let i2: I2 = {}
let i3: I3 = {}

// foo
arktest.assertEQ(foo(c2), "foo1")
arktest.assertEQ(foo(c1), "foo2")
arktest.assertEQ(foo(i1), "foo3")
arktest.assertEQ(foo(i2), "foo4")
arktest.assertEQ(foo(i3), "foo4")

// bar
arktest.assertEQ(bar(c2), "bar5")
arktest.assertEQ(bar(c1), "bar3")
arktest.assertEQ(bar(i1), "bar3")
arktest.assertEQ(bar(i2), "bar4")
arktest.assertEQ(bar(i3), "bar5")
