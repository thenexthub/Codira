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

function main(): void {
    const arr = new Array<number>()
    arktest.assertEQ( arr.length, 0)
    for (let i = 0; i < 30; i++) {
        arktest.assertEQ( arr.length, i)
        arr.push(i)
    }
    for (let i = 0; i < 30; i++) {
        arktest.assertTrue( arr.includes(i, undefined))
    }
    try {
        arr.at(50)
    } catch (e) {
        arktest.assertEQ(e.message, "Invalid index");
    }
    arktest.assertEQ( arr.length, 30)
    for (let i = 0; i < 30; i++) {
        arktest.assertEQ( arr.pop()!, 30 - 1 - i)
    }
    arktest.assertEQ( arr.length, 0)

    test_reverse()
    test_splice_1()
    test_unshift()
}

function test_reverse(): void {
    const arr = new Array<number>()
    arktest.assertEQ( arr.length, 0)
    for (let i = 0; i < 30; i++) {
        arktest.assertEQ( arr.length, i)
        arr.push(i)
    }
    arr.reverse()
    for (let i = 0; i < 30; i++) {
        arktest.assertEQ( arr.indexOf(i, undefined), 30 - 1 - i)
    }
}

function test_splice_1(): void {
    const arr = new Array<number>()
    for (let i = 0; i <= 3; i++) {
        arr.push(i)
    }
    const other = arr.splice(1)
    arktest.assertEQ( arr.length, 1)
    arktest.assertEQ( arr.indexOf(0, undefined), 0)

    arktest.assertEQ( other.length, 3)
    for (let i = 1; i <= 3; i++) {
        arktest.assertEQ( other.indexOf(i, undefined), i - 1)
    }

    other.splice(1, undefined, 6, 7, 8);
    arktest.assertEQ( other.length, 6)
    arktest.assertEQ( other.$_get(0), 1)
    arktest.assertEQ( other.$_get(1), 6)
    arktest.assertEQ( other.$_get(2), 7)
    arktest.assertEQ( other.$_get(3), 8)
    arktest.assertEQ( other.$_get(4), 2)
    arktest.assertEQ( other.$_get(5), 3)
}

function test_unshift(): void {
    const a = new Array<number>()
    a.unshift(1, 2, 3)
    arktest.assertEQ( a.length, 3)
    a.unshift(4, 5, 6)
    a.unshift(7, 8, 9)
    arktest.assertEQ( a.length, 9)
    arktest.assertEQ( a.$_get(0), 7)
    arktest.assertEQ( a.$_get(1), 8)
    arktest.assertEQ( a.$_get(2), 9)
    arktest.assertEQ( a.$_get(3), 4)
    arktest.assertEQ( a.$_get(4), 5)
    arktest.assertEQ( a.$_get(5), 6)
    arktest.assertEQ( a.$_get(6), 1)
    arktest.assertEQ( a.$_get(7), 2)
    arktest.assertEQ( a.$_get(8), 3)
}
