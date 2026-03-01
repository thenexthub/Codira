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

import ns from "testDecl"

function foo(i: ns.I) {
    arktest.assertEQ(i.i, "zxcv")
}

function testAll() {
    arktest.assertEQ(ns.a.i, "1234")
    arktest.assertEQ(ns.a.foo("abcd"), "abcd")
    arktest.assertEQ(ns.bar(123321), 123321)
    let a = new ns.A()
    a.i = "zxcv"
    foo(a)
}