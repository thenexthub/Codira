/*
* Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
*/

import { a, b, c, d, bar } from "testDecl";
import { e, n } from "testDecl";
import { Greeting, h, g } from "testDecl";
import { Color, UserRole } from "testDecl";
import { union, u } from "testDecl";
import { foo, C, iface } from "testDecl";
import { stringlist, numberlist } from "testDecl";

function TestGeneralPurposeTypes1() {
    arktest.assertEQ(a, 42);
    arktest.assertEQ(b, "Hi");
    arktest.assertEQ(c, 42n);
    arktest.assertEQ(d, true);
    arktest.assertEQ(bar(), 123);
    let foo = bar
    arktest.assertEQ(foo(), 123);
}

function TestGeneralPurposeTypes2() {
    arktest.assertEQ(typeof e, "number");
    arktest.assertEQ(typeof n, "number");
}

function TestGeneralPurposeTypes3() {
    let g1: Greeting = "Hello" //ok
    let g2: Greeting = h //ok
    let g3: Greeting = g //ok
    arktest.assertEQ(g1, "Hello");
    arktest.assertEQ(g2, "Hello");
    arktest.assertEQ(g3, "Hello");
}

function TestGeneralPurposeTypes4() {
    let c1: Color = Color.Red // ok
    let ur: UserRole = UserRole.Admin // ok
    // let c2: Color = Color.Yellow // CTE, no Yellow
}

function TestGeneralPurposeTypes5() {
    let u1: union = 1 //ok
    let u2: union = "1" //ok
    // let u3: union = true //CTE
    let u4: union = u //ok
}

function TestGeneralPurposeTypes6() {
    arktest.assertEQ(foo(1), 1);
    arktest.assertEQ(C.foo(2), 2);
    arktest.assertEQ(new C().bar(3), 3);
}

function baz(i: iface) {
    i.foo(4) //ok
    i.foo() //ok
}

function TestGeneralPurposeTypes7() {
    let index = 1;
    for(let e of numberlist) {
        arktest.assertEQ(e, index);
        index++;
    }
    arktest.assertEQ(stringlist[0], "Hello world");
}

function testAll() {
    TestGeneralPurposeTypes1()
    TestGeneralPurposeTypes2()
    TestGeneralPurposeTypes3()
    TestGeneralPurposeTypes4()
    TestGeneralPurposeTypes5()
    TestGeneralPurposeTypes6()
    TestGeneralPurposeTypes7()
}