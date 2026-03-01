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

let str: String = 'abcΣΨΩ0123456789!';
let bytes: byte[] = [97, 98, 99, -93, -88, -87, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 33]
let strTree : String = 'abcΣΨΩ0'.concat('123456789!');
let strSliced: String = 'sub_abcΣΨΩ0123456789!_sub'.substring(4, 21);

function equals(val: byte[], exp: byte[], offset: int): boolean {
    if (val.length != (exp.length - offset)) {
        return false;
	}
    for (let i: int = 0; i < val.length; ++i) {
        if (val[i] != exp[i + offset]) {
            return false;
		}
    }
    return true;
}

function printArray(a: byte[], offset: int, name: String): void {
    console.println(name + ' length: ' + (a.length - offset))
    console.print(name + ' data:  [ ')
    for (let i: int = 0; i < a.length; ++i) {
        console.print(a[i + offset] + ' ');
    }
    console.println(']');
}

function main(): int {
    let len: int = str.getLength()
    for (let i: int = 0; i < len; ++i) {
        let arr: byte[] = str.getBytes(i, len);
        if (!equals(arr, bytes, i)) {
            console.println('at: ' + i);
            printArray(arr, 0, 'actual  ')
            printArray(bytes, i, 'expected')
            return 1;
        }
    }

    arktest.assertEQ(strTree, str, 'strTree != str');
    arktest.assertEQ(strSliced, str, 'strSliced != str');

    for (let i: int = 0; i < len; ++i) {
        let arr: byte[] = strTree.getBytes(i, len);
        if (!equals(arr, bytes, i)) {
            console.println('at: ' + i);
            printArray(arr, 0,   'actual  ')
            printArray(bytes, i, 'expected')
            return 1;
        }
    }

    for (let i: int = 0; i < len; ++i) {
        let arr: byte[] = strSliced.getBytes(i, len);
        if (!equals(arr, bytes, i)) {
            console.println('at: ' + i);
            printArray(arr, 0,   'actual  ')
            printArray(bytes, i, 'expected')
            return 1;
        }
    }

    return 0;
}
