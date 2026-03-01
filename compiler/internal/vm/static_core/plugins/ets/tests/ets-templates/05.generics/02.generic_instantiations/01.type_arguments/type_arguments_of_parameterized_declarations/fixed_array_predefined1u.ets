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

// declare object with union
let v_char: A<char>|undefined
let v_boolean: A<boolean>|undefined
let v_byte: A<byte>|undefined
let v_short: A<short>|undefined
let v_int: A<int>|undefined
let v_long: A<long>|undefined
let v_float: A<float>|undefined
let v_double: A<double>|undefined

function test_double() {
    arktest.assertTrue(v_double instanceof A|undefined, "expect particular instance of v (double)")
    let v1 = new A<double>
    let fa: FixedArray<double> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (double)")
}

function test_float() {
    arktest.assertTrue(v_float instanceof A|undefined, "expect particular instance of v (float)")
    let v1 = new A<float>
    let fa: FixedArray<float> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (float)")
}

function test_long() {
    arktest.assertTrue(v_long instanceof A|undefined, "expect particular instance of v (long)")
    let v1 = new A<long>
    let fa: FixedArray<long> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (long)")
}

function test_int() {
    arktest.assertTrue(v_int instanceof A|undefined, "expect particular instance of v (int)")
    let v1 = new A<int>
    let fa: FixedArray<int> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (int)")
}

function test_short() {
    arktest.assertTrue(v_short instanceof A|undefined, "expect particular instance of v (short)")
    let v1 = new A<short>
    let fa: FixedArray<short> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (short)")
}

function test_byte() {
    arktest.assertTrue(v_byte instanceof A|undefined, "expect particular instance of v (byte)")
    let v1 = new A<byte>
    let fa: FixedArray<byte> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (byte)")
}

function test_boolean() {
    arktest.assertTrue(v_boolean instanceof A|undefined, "expect particular instance of v (boolean)")
    let v1 = new A<boolean>
    let fa: FixedArray<boolean> = []
    arktest.assertTrue(v1.foo(fa), "expect successfully invoking a class method (boolean)")
}

function test_char() {
    arktest.assertTrue(v_char instanceof A|undefined, "expect particular instance of v (char)")
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
