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

enum Color {
    red = 0,
    green = 1,
    blue = 2,
    gray = 5
}

enum AlphaBeta {
    a = 'a',
    b = 'b'
}

function compareWithColor(a: number | Color, b: number) {
    return a == b
}

function compareWithColorStrict(a: number | Color, b: number) {
    return a === b
}

function compareWithString(a: string | AlphaBeta, b: string) {
    return a == b
}

function compareWithStringStrict(a: string | AlphaBeta, b: string) {
    return a === b
}

function main() {
    arktest.assertTrue(compareWithColor(Color.red, 0))
    arktest.assertTrue(compareWithColor(Color.green, 1))
    arktest.assertTrue(compareWithColor(Color.blue, 2))
    arktest.assertTrue(compareWithColor(Color.gray, 5))
    arktest.assertTrue(compareWithString(AlphaBeta.a, 'a'))
    arktest.assertTrue(compareWithString(AlphaBeta.b, 'b'))

    arktest.assertTrue(!compareWithColor(Color.red, 1))
    arktest.assertTrue(!compareWithString(AlphaBeta.a, 'b'))
}
