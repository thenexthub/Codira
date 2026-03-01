/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


function test_set_bool_double_overrided1() {
    let set: Set<boolean> = new Set<boolean>()
    set.add(false)
    arktest.assertFalse(__noinline__SetHas_bool(set))
    arktest.assertFalse(__noinline__SetDelete_bool(set))
}

function __noinline__SetHas_bool(set: Set<boolean>): boolean {
    return set.has(true);
}
function __noinline__SetDelete_bool(set: Set<boolean>): boolean {
    return set.delete(true);
}



function test_set_bool() {
    let set: Set<boolean> = new Set<boolean>
    set.add(false)
    arktest.assertTrue(set.has(false))
    arktest.assertFalse(set.has(true))

    set.add(true)
    arktest.assertTrue(set.delete(true))
    arktest.assertFalse(set.delete(true))
}

function test_set_char() {
    let set: Set<char> = new Set<char>
    set.add(c'A')
    arktest.assertTrue(set.has(c'A'))
    arktest.assertFalse(set.has(c'B'))

    set.add(c'B')
    arktest.assertTrue(set.delete(c'B'))
    arktest.assertFalse(set.delete(c'B'))

    set.add(c'\u012C')
    arktest.assertTrue(set.has(c'\u012C'))
    arktest.assertFalse(set.has(c'\u002C'))
    arktest.assertFalse(set.has(c'\u0001'))
}

function test_set_byte() {
    let set: Set<byte> = new Set<byte>
    set.add((1).toByte())
    arktest.assertTrue(set.has((1).toByte()))
    arktest.assertFalse(set.has((2).toByte()))

    set.add((2).toByte())
    arktest.assertTrue(set.delete((2).toByte()))
    arktest.assertFalse(set.delete((2).toByte()))
}

function test_set_short() {
    let set: Set<short> = new Set<short>
    set.add((1).toShort())
    arktest.assertTrue(set.has((1).toShort()))
    arktest.assertFalse(set.has((2).toShort()))

    set.add((2).toShort())
    arktest.assertTrue(set.delete((2).toShort()))
    arktest.assertFalse(set.delete((2).toShort()))
}

function test_set_int(): void {
    let set: Set<int> = new Set<int>
    set.add(1)
    arktest.assertTrue(set.has(1))
    arktest.assertFalse(set.has(2))

    set.add(2)
    arktest.assertTrue(set.delete(2))
    arktest.assertFalse(set.delete(2))
}

function test_set_long() {
    let set: Set<long> = new Set<long>
    set.add(1)
    arktest.assertTrue(set.has(1))
    arktest.assertFalse(set.has(2))

    set.add(2)
    arktest.assertTrue(set.delete(2))
    arktest.assertFalse(set.delete(2))
}

function test_set_float() {
    let set: Set<float> = new Set<float>
    set.add(7.2f)
    arktest.assertTrue(set.has(7.2f))
    arktest.assertFalse(set.has(2))

    set.add(2.5f)
    arktest.assertTrue(set.delete(2.5f))
    arktest.assertFalse(set.delete(2.5f))
}

function test_set_double() {
    let set: Set<double> = new Set<double>
    set.add(1.5)
    arktest.assertTrue(set.has(1.5))
    arktest.assertFalse(set.has(2.1))

    set.add(2.1)
    arktest.assertTrue(set.delete(2.1))
    arktest.assertFalse(set.delete(2.1))
}

function test_set_string() {
    let set: Set<string> = new Set<string>
    arktest.assertFalse(set.has(""))
    set.add("")
    set.add("abc")
    set.add("defgh")
    arktest.assertTrue(set.has(""))
    arktest.assertTrue(set.has("abc"))
    arktest.assertTrue(set.has("defgh"))
    arktest.assertFalse(set.has("erty"))

    set.add("erty")
    arktest.assertTrue(set.delete("erty"))
    arktest.assertFalse(set.delete("erty"))
}

function main(): void {
    test_set_bool()
    test_set_char()
    test_set_byte()
    test_set_short()
    test_set_int()
    test_set_long()
    test_set_float()
    test_set_double()
    test_set_string()
}
