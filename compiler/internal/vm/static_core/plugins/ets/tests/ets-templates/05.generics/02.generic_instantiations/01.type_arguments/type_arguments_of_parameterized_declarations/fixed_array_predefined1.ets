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
    foo(p: FixedArray<T>) {
        return true
    }
}

// new object
let v_char = new A<char>
let v_boolean = new A<boolean>
let v_byte = new A<byte>
let v_short = new A<short>
let v_int = new A<int>
let v_long = new A<long>
let v_float = new A<float>
let v_double = new A<double>

function test_double() {
    arktest.assertTrue(v_double instanceof A, "expect particular instance of v (double)")
    let v1 = new A<double>
    let fa: FixedArray<double> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (double)")
}

function test_float() {
    arktest.assertTrue(v_float instanceof A, "expect particular instance of v (float)")
    let v1 = new A<float>
    let fa: FixedArray<float> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (float)")
}

function test_long() {
    arktest.assertTrue(v_long instanceof A, "expect particular instance of v (long)")
    let v1 = new A<long>
    let fa: FixedArray<long> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (long)")
}

function test_int() {
    arktest.assertTrue(v_int instanceof A, "expect particular instance of v (int)")
    let v1 = new A<int>
    let fa: FixedArray<int> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (int)")
}

function test_short() {
    arktest.assertTrue(v_short instanceof A, "expect particular instance of v (short)")
    let v1 = new A<short>
    let fa: FixedArray<short> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (short)")
}

function test_byte() {
    arktest.assertTrue(v_byte instanceof A, "expect particular instance of v (byte)")
    let v1 = new A<byte>
    let fa: FixedArray<byte> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (byte)")
}

function test_boolean() {
    arktest.assertTrue(v_boolean instanceof A, "expect particular instance of v (boolean)")
    let v1 = new A<boolean>
    let fa: FixedArray<boolean> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (boolean)")
}

function test_char() {
    arktest.assertTrue(v_char instanceof A, "expect particular instance of v (char)")
    let v1 = new A<char>
    let fa: FixedArray<char> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (char)")
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
