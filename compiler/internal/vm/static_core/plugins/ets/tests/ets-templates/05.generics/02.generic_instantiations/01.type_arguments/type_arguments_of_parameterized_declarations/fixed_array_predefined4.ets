/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Verify instantiation of the FixedArray with the predefined value type.
---*/

class A<T> {
    // class method with desired return type
    foo_a(): FixedArray<T> {
        // Note new FixedArray<T> would throw CTE per spec 17.2
        let res: FixedArray<T> = []
        return res
    }
}

function foo_char(p: A<char>) { return p instanceof A }
function foo_boolean(p: A<boolean>) { return p instanceof A }
function foo_byte(p: A<byte>) { return p instanceof A }
function foo_short(p: A<short>) { return p instanceof A }
function foo_int(p: A<int>) { return p instanceof A }
function foo_long(p: A<long>) { return p instanceof A }
function foo_float(p: A<float>) { return p instanceof A }
function foo_double(p: A<double>) { return p instanceof A }

function test_double() {
    let param = new A<double>
    arktest.assertTrue(foo_double(param), "expect successfully invoking a function (double)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (double)")
}

function test_float() {
    let param = new A<float>
    arktest.assertTrue(foo_float(param), "expect successfully invoking a function (float)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (float)")
}

function test_long() {
    let param = new A<long>
    arktest.assertTrue(foo_long(param), "expect successfully invoking a function (long)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (long)")
}

function test_int() {
    let param = new A<int>
    arktest.assertTrue(foo_int(param), "expect successfully invoking a function (int)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (int)")
}

function test_short() {
    let param = new A<short>
    arktest.assertTrue(foo_short(param), "expect successfully invoking a function (short)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (short)")
}

function test_byte() {
    let param = new A<byte>
    arktest.assertTrue(foo_byte(param), "expect successfully invoking a function (byte)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (byte)")
}

function test_boolean() {
    let param = new A<boolean>
    arktest.assertTrue(foo_boolean(param), "expect successfully invoking a function (boolean)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (boolean)")
}

function test_char() {
    let param = new A<char>
    arktest.assertTrue(foo_char(param), "expect successfully invoking a function (char)")
    arktest.assertTrue(param.foo_a() instanceof FixedArray, "expect successfully invoking a class method (char)")
}

function main() {
    test_double()
    test_float()
    test_long()
    test_int()
    test_short()
    test_byte()
    test_boolean()
    test_char()
}
