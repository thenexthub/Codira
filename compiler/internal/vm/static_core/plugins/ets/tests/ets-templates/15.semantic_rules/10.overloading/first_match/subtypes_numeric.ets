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

// from less specific to more
function foo(p: double): string { return "foo1" }
function foo(p: float): string { return "foo2" }
function foo(p: long): string { return "foo3" }
function foo(p: int): string { return "foo4" }
function foo(p: short): string { return "foo5" }
function foo(p: byte): string { return "foo6" }

// from more specific to less
function bar(p: byte): string { return "bar1" }
function bar(p: short): string { return "bar2" }
function bar(p: int): string { return "bar3" }
function bar(p: long): string { return "bar4" }
function bar(p: float): string { return "bar5" }
function bar(p: double): string { return "bar6" }

let pByte: byte = 1
let pShort: short = 1
let pLong: long = 1
let pInt: int = 1

// from less specific to more
arktest.assertEQ(foo(pByte), "foo1")
arktest.assertEQ(foo(pShort), "foo1")
arktest.assertEQ(foo(1), "foo1")
arktest.assertEQ(foo(pLong), "foo1")
arktest.assertEQ(foo(1.1f), "foo1")
arktest.assertEQ(foo(1.1), "foo1")

// from more specific to less
arktest.assertEQ(bar(pByte), "bar1")
arktest.assertEQ(bar(pShort), "bar2")
arktest.assertEQ(bar(pInt), "bar3")
arktest.assertEQ(bar(pLong), "bar4")
arktest.assertEQ(bar(1.1f), "bar5")
arktest.assertEQ(bar(1.1), "bar6")
