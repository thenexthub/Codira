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

import {launch} from 'std/concurrency'
import {Job} from 'std/core'

// simple fast pseudo-random generator
function xorshift(state: long): long {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

// custom parse because Number.parseLong uses double as intermediate value,
// and BigInt(s) is too slow
function parseLong(s: String): long|null {
    if (s.length == 0) {
        return null;
    }
    if (s == '0') {
        return 0;
    }
    let negative = s.charAt(0) == c'-';
    let result: long = 0;
    let startFrom = negative ? 1 : 0;
    if (startFrom >= s.length || s.charAt(startFrom) == c'0') {
        return null;
    }
    for (let i = startFrom; i < s.length; i++) {
        let ch = s.charAt(i);
        if (!Char.isDecDigit(ch)) {
            return null;
        }
        let digit = (ch.toInt() - Char.toInt(c'0')).toInt();
        if (negative) {
            if (result < (Long.MIN_VALUE + digit) / 10) {
                return null;
            }
            // don't negate the whole number to avoid overflow for Long.MIN_VALUE
            digit = -digit;
        } else if (result > (Long.MAX_VALUE - digit) / 10) {
            return null;
        }
        result = result * 10 + digit;
    }
    return result;
}

function isEqual<T>(s: String, number: T): boolean {
    if (number instanceof Long) {
        let parsed = parseLong(s);
        return number.equals(parsed);
    }
    if (number instanceof Double) {
        if (Double.isNaN(number)) {
            return s == 'NaN';
        }
        let eps = Math.abs(number.toDouble()) * 2 * Double.EPSILON;
        if (eps < 1e-307) {
            // parsing of small doubles is inaccurate by now
            eps = Math.abs(number.toDouble()) * (1e4 * Double.EPSILON);

            if (eps == 0) {
                eps = 4.9e-308;
            }
        }
        let parsed = Double.parseFloat(s);
        if (Math.abs(parsed - number) > eps) {
            console.println(`double parsed: ${parsed}, bitCastToLong: ${Double.bitCastToLong(parsed)}, eps: ${eps}, delta: ${parsed - number}`)
            console.println(`number: ${number.toDouble()}, bitCastToLong: ${Double.bitCastToLong(number.toDouble())}`)
            return false;
        }
        return true;
    }
    if (number instanceof Float) {
        if (Float.isNaN(number)) {
            return s == 'NaN';
        }
        let eps: float = (Math.abs(number.toFloat()) * 2 * Float.EPSILON).toFloat();
        let parsed = Double.parseFloat(s).toFloat();
        let delta: float = parsed - number.toFloat();
        if (Math.abs(delta) > eps) {
            console.println(`float parsed: ${parsed}, bitCastToInt: ${Float.bitCastToInt(parsed as float)} eps: ${eps} delta: ${delta}`)
            console.println(`number: ${number.toFloat()}, bitCastToInt: ${Float.bitCastToInt(number.toFloat())}`)
            return false;
        }
        return true;
    }
    throw new Error('Unknown type: ' + number);
}

class Worker<T> {
    values: T[];
    constructor(values: T[]) {
        // wrap into class because llvm backend does not support launch with ref arguments
        this.values = values;
    }

    work(state: long, iters: int, check: boolean): string {
        let strings = new Array<String>;
        let numbers = new Array<T>;
        for (let i = 0; i < iters; i++) {
            state = xorshift(state);
            let randomInt = (state >>> 33).toInt();
            let index = randomInt % this.values.length;
            let value = this.values[index]!;
            let str = (value as object).toString();
            if (check) {
                // arktest.assertTrue(isEqual(str, value), `not equal: string ${str}, number ${value}`)
                strings.push(str);
                numbers.push(value);
            }
        }
        // defer check to increase cache pressure
        if (check) {
            for (let i = 0; i < iters; i++) {
                let expected = (numbers[i]! as object).toString();
                arktest.assertEQ(strings[i], expected, `results of toString differ: expected ${expected}, got ${strings[i]}`);
                arktest.assertTrue(isEqual(strings[i], numbers[i]), `not equal: string ${strings[i]}, number ${numbers[i]}`)
            }
        }
        return 'OK';
    }
}

const workerCount = 10;
const valueCount = 500;
const iterCount = valueCount * 3;

function doTest<T>(values: T[], workers: int) {
    let jobs = new (Job<String> | undefined)[workers];
    for (let i = 0; i < workers; i++) {
        let state = (i.toLong() * 1234567) | 42;
        // check only in one worker to make others faster
        let check = i == 0;
        let worker = new Worker<T>(values);
        const work = (w: Worker<T>, s: long, i: int, c: boolean) => { 
            return w.work(s, i, c);
        }
        jobs[i] = launch<string, (w: Worker<T>, s: long, i: int, c: boolean) => string>(work, worker, state, iterCount, check);
    }
    for (let i = 0; i < workers; i++) {
        let res = jobs[i]!.Await();
        arktest.assertEQ(res, 'OK');
    }
}

function testDoubles() {
    // console.println('testDoubles')
    let values = new Double[valueCount];
    for (let i = 0; i < valueCount; i++) {
        let exp = (Math.random() * 2 - 1) * (310).toInt()
        values[i] = Math.random() * Math.pow(10, exp);
    }
    doTest(values, workerCount);
}

function testDoublesBitcast() {
    // console.println('testDoublesBitcast');
    let values = new Double[valueCount];
    let state = 42 as long;
    for (let i = 0; i < valueCount; i++) {
        state = xorshift(state);
        values[i] = Double.bitCastFromLong(state);
    }
    doTest(values, workerCount);
}

function testFloats() {
    // console.println('testFloats');
    let values = new Float[valueCount];
    for (let i = 0; i < valueCount; i++) {
        let exp = (Math.random() * 2 - 1) * (39).toInt()
        values[i] = (Math.random() * Math.pow(10, exp)).toFloat();
    }
    doTest(values, workerCount);
}

function testFloatsBitcast() {
    // console.println('testFloatsBitcast');
    let values = new Float[valueCount];
    let state = 42 as long;
    for (let i = 0; i < valueCount; i++) {
        state = xorshift(state);
        values[i] = Float.bitCastFromInt(state.toInt());
    }
    doTest(values, workerCount);
}

function testLongs() {
    // console.println('testLongs');
    let values = new Long[valueCount];
    let state = 42 as long;
    for (let i = 0; i < valueCount; i++) {
        state = xorshift(state);
        values[i] = state;
    }
    doTest(values, workerCount);
}

function main() {
    testDoubles();
    testDoublesBitcast();
    testFloats();
    testFloatsBitcast();
    testLongs();
}
