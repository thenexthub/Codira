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

class ZeroDivisor extends Error {}
function divide(a: number, b: number): number {
    if (b == 0) {
        throw new ZeroDivisor();
    }
    return a / b;
}
function process(a: number, b: number): number {
    try {
        let res = divide(a, b);
        return res;
    } catch (err) {
        return (err instanceof ZeroDivisor) ? -1 : 0;
    }
}
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1 ', process(4, 2) === 2);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test1 ', process(4, 0) === -1);

class SomeResource {
    close() {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('SomeResource close');
    }
}
function processFile(name: string) {
    let r = new SomeResource();
    try {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('open ' + name);
    } finally {
        r.close();
    }
}
processFile('test2 file');

class UnknownError extends Error { // User-defined error class
    public error: Error
    constructor(error: Error) {
        super()
        this.error = error
    }
}
function getArrayElement<T>(array: T[], index: int): T | null {
    try {
        if (index === 0 ){
            throw new Error('0')
        }
        return array[index] // access array
    } catch (e) {
        if (e instanceof RangeError) {// invalid index detected
            return null
        }
        throw new UnknownError(e as Error) // Unknown error occurred
    }
}

let arr: string[] = ['abc', 'xyz', 'zxc'];
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test3 ', getArrayElement(arr, 2) === 'zxc');
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('test3 ', getArrayElement(arr, 4) === null);
try {
    getArrayElement(arr, 0);
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('test3 ', 'not reached');
} catch (e) {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('test3 ', e instanceof UnknownError);
}
