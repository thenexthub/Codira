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

const DUMP: boolean = false;
const HITS_EXPECTED: int = 21;
let golden: String = 'abcΣΨΩ0123456789!';
let hits: int = 0;
let silver: String = 'abcΣΨΩ'.concat('0123456789!');  // TreeString
let copper: String = 'sub_abcΣΨΩ0123456789!_sub'.substring(4, 21);  // SlicedString

let table1: char[][] = [
    [c'a', c'b', c'c', c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'b', c'c', c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'c', c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'Σ', c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'Ψ', c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'Ω', c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'0', c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'1', c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'2', c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'3', c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'4', c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'5', c'6', c'7', c'8', c'9', c'!'],
    [c'6', c'7', c'8', c'9', c'!'],
    [c'7', c'8', c'9', c'!'],
    [c'8', c'9', c'!'],
    [c'9', c'!'],
    [c'!'],
    []
];

function printArray(a: char[], name: String): void
{
    console.println(name + ' length: ' + a.length)
    console.print(name + ' data:   [')
    for (let n: int = 0; n < a.length; ++n) {
        console.print(a[n]);
    }
    console.println(']');
}

function equals(lhs: char[], rhs: char[]): boolean
{
    if (lhs.length != rhs.length)
        return false;
    for (let i: int = 0; i < lhs.length; ++i) {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}

function main(): int {

    for (let i: int = 0; i <= golden.getLength(); ++i) {
        let arr: char[] = golden.getChars(i, golden.getLength());
        if (equals(arr, table1[i])) {
            ++hits;
        }
        if (DUMP) {
            printArray(table1[i], 'expected');
            printArray(arr, 'getChars');
            console.println();
        }
    }

    try {
        // end > length
        let arr: char[] = golden.getChars(1, golden.getLength() + 1);
    } catch (e) {
        ++hits;
        if (DUMP) {
            console.println('end > length');
            console.println(e);
        }
    }

    try {
        // begin > end
        let arr: char[] = golden.getChars(2, 1);
    } catch (e) {
        ++hits;
        if (DUMP) {
            console.println('begin > end');
            console.println(e);
        }
    }

    try {
        // begin < 0
        let arr: char[] = golden.getChars(-1, 1);
    } catch (e) {
        ++hits;
        if (DUMP) {
            console.println('begin < 0');
            console.println(e);
        }
    }

    arktest.assertTrue(golden=== silver);
    arktest.assertTrue(golden=== copper);
    // test for TreeString
    for (let i: int = 0; i <= silver.getLength(); ++i) {
        let arr: char[] = silver.getChars(i, silver.getLength());
        if (equals(arr, table1[i])) {
            ++hits;
        }
        if (DUMP) {
            printArray(table1[i], 'expected');
            printArray(arr, 'getChars');
            console.println();
        }
    }

    try {
        // end > length
        let arr: char[] = silver.getChars(1, silver.getLength() + 1);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            ++hits;
            if (DUMP) {
                console.println('end > length');
                console.println(e);
            }
        }
    }

    try {
        // begin > end
        let arr: char[] = silver.getChars(2, 1);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            ++hits;
            if (DUMP) {
                console.println('begin > end');
                console.println(e);
            }
        } else {
            throw e;
        }
    }

    try {
        // begin < 0
        let arr: char[] = silver.getChars(-1, 1);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            ++hits;
            if (DUMP) {
                console.println('begin < 0');
                console.println(e);
            }
        } else {
            throw e;
        }
    }
    // test for SlicedString
    for (let i: int = 0; i <= copper.getLength(); ++i) {
        let arr: char[] = copper.getChars(i, copper.getLength());
        if (equals(arr, table1[i])) {
            ++hits;
        }
        if (DUMP) {
            printArray(table1[i], 'expected');
            printArray(arr, 'getChars');
            console.println();
        }
    }

    try {
        // end > length
        let arr: char[] = copper.getChars(1, copper.getLength() + 1);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            ++hits;
            if (DUMP) {
                console.println('end > length');
                console.println(e);
            }
        } else {
            throw e;
        }
    }

    try {
        // begin > end
        let arr: char[] = copper.getChars(2, 1);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            ++hits;
            if (DUMP) {
                console.println('begin > end');
                console.println(e);
            }
        } else {
            throw e;
        }
    }

    try {
        // begin < 0
        let arr: char[] = copper.getChars(-1, 1);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            ++hits;
            if (DUMP) {
                console.println('begin < 0');
                console.println(e);
            }
        } else {
            throw e;
        }
    }


    if (hits != 3*HITS_EXPECTED) {
        if (DUMP) {
            console.println('hits: ' + hits + ' (expected ' + HITS_EXPECTED + ')');
        }
        console.println('FAIL');
        return 1;
    }
    return 0;
}
