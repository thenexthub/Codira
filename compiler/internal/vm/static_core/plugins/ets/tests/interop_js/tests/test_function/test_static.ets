/*
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
*/

import { X, Y, foo1, foo2, foo3, foo4, bar } from "testDecl";

function TestFunction1() {
    arktest.assertEQ(foo1("1"), "1")
    let x = new X()
    x.s = "2"
    arktest.assertEQ(foo2(x), "2")
    let value = 1
    for (let e of foo3(1, 2, 3, 4, 5, 6)) {
        arktest.assertEQ(e, value)
        value++
    }
}

function TestFunction2() {
    arktest.assertEQ(bar("1"), "1")
    arktest.assertEQ(bar(3.1), 3.1)
    let value = 1.1
    for (let e of bar([1.1, 2.1, 3.1])) {
        arktest.assertEQ(e, value)
        value++
    }
}

function TestFunction3() {
    let ret = Y.fooArgs(10.0, 20.0, 30.0)
    arktest.assertEQ(ret, 10.0)
    let x = new X(1, 2, 3, 4)
    arktest.assertEQ(x.n, 10.0)
}

function TestFunction4() {
    arktest.assertEQ(foo4(), 2)
    arktest.assertEQ(foo4(2), 3)
}

function testAll() {
    TestFunction1()
    TestFunction2()
    TestFunction3()
    TestFunction4()
}