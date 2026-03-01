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

function isEqual<T>(a: T, b: T): boolean {
    return a == b;
}

function erase<T>(v: T): Any {
    return v;
}

enum Color {
    RED,
    GREEN,
    BLUE = 2147483647,
    YELLOW = -2147483648,
}

enum Color64 {
    RED,
    GREEN,
    BLUE = 2147483647,
    YELLOW = -2147483648,
    PURPLE = 9223372036854775807,
    PINK = -9223372036854775808
}

enum ColorString {
    RED = '',
    GREEN = 'GREEN'
}

function testNormal() {
    arktest.assertTrue(isEqual(Color.RED, Color.RED));
    arktest.assertTrue(isEqual(Color.GREEN, Color.GREEN));
    arktest.assertTrue(isEqual(Color.BLUE, Color.BLUE));
    arktest.assertTrue(isEqual(Color.YELLOW, Color.YELLOW));

    arktest.assertTrue(isEqual(Color64.RED, Color64.RED));
    arktest.assertTrue(isEqual(Color64.GREEN, Color64.GREEN));
    arktest.assertTrue(isEqual(Color64.BLUE, Color64.BLUE));
    arktest.assertTrue(isEqual(Color64.YELLOW, Color64.YELLOW));
    arktest.assertTrue(isEqual(Color64.PURPLE, Color64.PURPLE));
    arktest.assertTrue(isEqual(Color64.PINK, Color64.PINK));

    arktest.assertTrue(isEqual(ColorString.RED, ColorString.RED));
    arktest.assertTrue(isEqual(ColorString.GREEN, ColorString.GREEN));
    
    arktest.assertTrue(!isEqual(Color.RED, Color.GREEN));
    arktest.assertTrue(!isEqual(Color.RED, Color.BLUE));
    arktest.assertTrue(!isEqual(Color.RED, Color.YELLOW));
    arktest.assertTrue(!isEqual(ColorString.RED, ColorString.GREEN));
}

function testErase() {
    arktest.assertTrue(isEqual(erase(Color.RED), erase(Color.RED)));
    arktest.assertTrue(isEqual(erase(Color.GREEN), erase(Color.GREEN)));
    arktest.assertTrue(isEqual(erase(Color.BLUE), erase(Color.BLUE)));
    arktest.assertTrue(isEqual(erase(Color.YELLOW), erase(Color.YELLOW)));

    arktest.assertTrue(isEqual(erase(Color64.RED), erase(Color64.RED)));
    arktest.assertTrue(isEqual(erase(Color64.GREEN), erase(Color64.GREEN)));
    arktest.assertTrue(isEqual(erase(Color64.BLUE), erase(Color64.BLUE)));
    arktest.assertTrue(isEqual(erase(Color64.YELLOW), erase(Color64.YELLOW)));
    arktest.assertTrue(isEqual(erase(Color64.PURPLE), erase(Color64.PURPLE)));
    arktest.assertTrue(isEqual(erase(Color64.PINK), erase(Color64.PINK)));

    arktest.assertTrue(isEqual(erase(Color64.RED), erase(Color.RED)));
    arktest.assertTrue(isEqual(erase(Color64.GREEN), erase(Color.GREEN)));
    arktest.assertTrue(isEqual(erase(Color64.BLUE), erase(Color.BLUE)));
    arktest.assertTrue(isEqual(erase(Color64.YELLOW), erase(Color.YELLOW)));

    arktest.assertTrue(isEqual(erase(ColorString.RED), erase(ColorString.RED)));
    arktest.assertTrue(isEqual(erase(ColorString.GREEN), erase(ColorString.GREEN)));

    arktest.assertTrue(!isEqual(erase(Color.RED), erase(Color.GREEN)));
    arktest.assertTrue(!isEqual(erase(Color.RED), erase(Color.BLUE)));
    arktest.assertTrue(!isEqual(erase(Color.RED), erase(Color.YELLOW)));
    arktest.assertTrue(!isEqual(erase(ColorString.RED), erase(ColorString.GREEN)));
    arktest.assertTrue(!isEqual(erase(Color.BLUE), erase(Color64.PURPLE)));
    arktest.assertTrue(!isEqual(erase(Color.YELLOW), erase(Color64.PINK)));
}

function main() {
    testNormal();
    testErase();
}
