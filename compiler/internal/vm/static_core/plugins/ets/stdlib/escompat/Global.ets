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

package escompat;

/**
 * Represents the NaN value according to IEEE-754 specification
 */
// initialized in _initializerBlock_.ets
export const NaN: number;

/**
 * Represents the +Infinity value according to IEEE-754 specification
 */
// initialized in _initializerBlock_.ets
export const Infinity: number;

/**
 * Parses integer number from `String` with specified radix
 *
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @returns result of parsing
 */
export function parseInt(s: String): number {
    return Double.parseInt(s);
}

/**
 * Parses integer number from `String` with specified radix
 *
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @returns result of parsing
 */
export function parseInt(s: String, radix: int): number {
    return Double.parseInt(s, radix);
}

/**
 * Parses integer number from `String` with specified radix
 *
 * This function is an equivalent to @link{<Double.parseInt(s, radix)>}
 *
 * @returns result of parsing
 */
export function parseInt(s: String, radix: number): number {
    return Double.parseInt(s, radix);
}

/**
 * Parses floating point number from `String`
 *
 * This function is an equivalent to @link{<Double.parseFloat(s)>}
 *
 * @returns result of parsing
 */
export function parseFloat(s: String): number {
    return Double.parseFloat(s);
}

/**
 * Checks if `byte` value is `NaN` (not a number)
 *
 * @param b the `byte` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(b: byte): boolean { return false; }

/**
 * Checks if `short` value is `NaN` (not a number)
 *
 * @param s the `short` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(s: short): boolean { return false; }

/**
 * Checks if `int` value is `NaN` (not a number)
 *
 * @param i the `int` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(i: int): boolean { return false; }

/**
 * Checks if `long` value is `NaN` (not a number)
 *
 * @param l the `long` value to test
 *
 * @returns unconditionally returns `false`. Integral types can not be `NaN`.
 */
export function isNaN(l: long): boolean { return false; }

/**
 * Checks if `float` value is `NaN` (not a number)
 *
 * @param f the `float` value to test
 *
 * @returns `true` if the argument is `NaN`, `false` otherwise.
 */
export function isNaN(f: float): boolean {
    return Float.isNaN(f);
}

/**
 * Checks if double is `NaN` (not a number)
 *
 * @param d the `double` to test
 *
 * @returns `true` if the argument is `NaN`, `false` otherwise.
 */
export function isNaN(d: number): boolean {
    return d.isNaN();
}

/**
 * Checks if `byte` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param b the `byte` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(b: byte): boolean { return true; }

/**
 * Checks if `short` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param s the `short` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(s: short): boolean { return true; }

/**
 * Checks if `int` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param i the `int` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(i: int): boolean { return true; }

/**
 * Checks if `long` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param l the `long` value to test
 *
 * @returns unconditionally returns `true`. Integral types can not be `NaN` or `Infinity`.
 */
export function isFinite(l: long): boolean { return true; }

/**
 * Checks if `Float` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param f the `Float` value to test
 *
 * @returns `true` if the argument is a floating point value, `false` otherwise.
 */
export function isFinite(f: float): boolean {
    return f.isFinite();
}

/**
 * Checks if `double` value is a floating point value (not a `NaN` or `Infinity`)
 *
 * @param d the `double` value to test
 *
 * @returns `true` if the argument is a floating point value, `false` otherwise.
 */
export function isFinite(d: number): boolean {
    return d.isFinite();
}

/**
 * Checks if `byte` is similar to an integer value
 *
 * @param b the `byte` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(b: byte): boolean { return true; }

/**
 * Checks if `short` is similar to an integer value
 *
 * @param s the `short` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(s: short): boolean { return true; }

/**
 * Checks if `int` is similar to an integer value
 *
 * @param i the `int` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(i: int): boolean { return true; }

/**
 * Checks if `long` value is similar to an integer value
 *
 * @param l the `long` value to test
 *
 * @returns boolean - unconditionally returns `true` as integral types are integer.
 */
export function isInteger(l: long): boolean { return true; }

/**
 * Checks if float is similar to an integer value
 *
 * @param v the float to test
 *
 * @returns boolean - true if the argument is similar to an integer value
 */
export function isInteger(v: float): boolean {
    return v.isInteger();
}

/**
 * Checks if double is similar to an integer value
 *
 * @param v the double to test
 *
 * @returns boolean - true if the argument is similar to an integer value
 */
export function isInteger(v: number): boolean {
    return v.isInteger();
}

/**
 * Checks if int can be represented in float without lossing precision
 *
 * @param v: int - the int to test
 *
 * @returns boolean - true if the argument is less than Float.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: int): boolean {
    return v <= Float.MAX_SAFE_INTEGER as int;
}

/**
 * Checks if long can be represented in double without lossing precision
 *
 * @param v: long - the long to test
 *
 * @returns boolean - true if the argument is less than Double.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: long): boolean {
    return v <= Double.MAX_SAFE_INTEGER.toLong();
}

/**
 * Checks if float is a safe integer value
 *
 * @param v: float - the float to test
 *
 * @returns boolean - true if the argument is integer and less than Float.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: float): boolean {
    return v.isSafeInteger();
}

/**
 * Checks if double is a safe integer value
 *
 * @param v the double to test
 *
 * @returns boolean - true if the argument is integer and less than Double.MAX_SAFE_INTEGER
 */
export function isSafeInteger(v: number): boolean {
    return v.isSafeInteger();
}

/**
 * Execute function on the next cycle.
 *
 * @param { Function } func - the function to be executed on the next cycle.
 * @returns { int } timer id
 */
export function setTimeout(func: Function): int {
    return setTimeout(func, 0);
}

/**
 * Execute function with parameters after the timer expires.
 *
 * @param { Function } func - the function to be executed after the timer expires.
 * @param { int | null | undefined}  delayMs - timeout in milliseconds, if pass null or undefined will be treated as 0 milliseconds.
 * @param { ...Any } args - parameters passed to func.
 * @returns { int } timer id
 */
export function setTimeout(func: Function, delayMs: int | null | undefined, ...args: FixedArray<Any>): int {
    if (delayMs) {
        return startTimer((): void => { func.unsafeCall(...args) }, delayMs, false);
    } else {
        return startTimer((): void => { func.unsafeCall(...args) }, 0, false);
    }
}

/**
 * Print error message after the timer expires.
 *
 * @param { string } msg - the message to be printed after the timer expires.
 * @param { int | null | undefined}  delayMs - timeout in milliseconds, if pass empty or null or undefined will be treated as 0 milliseconds.
 * @returns { int } timer id
 */
export function setTimeout(msg: string, delayMs?: int | null): int {
    if (delayMs) {
        return setTimeout(():void => {
            console.error(msg)
        }, delayMs);
    } else {
        return setTimeout(():void => {
            console.error(msg)
        }, 0);
    }
}

/**
 * Cancel the specified timer.
 *
 * @param { int | null | undefined} timerId - the id of the timer returned from setTimeout, if pass empty or null or undefined will do nothing.
 */
export function clearTimeout(timerId?: int | null): void {
    if (timerId) {
        stopTimer(timerId);
    }
}

/**
 * Repeatedly call the function with delayMs interval between calls. The first call will be after delayMs.
 *
 * @param { Function } func - the function to be executed.
 * @param { int | null | undefined} delayMs - timeout in milliseconds, if pass null or undefined will be treated as 0 milliseconds.
 * @param { ...Any } args - parameters passed to func.
 * @returns { int } - timer id
 */
export function setInterval(func: Function, delayMs: int | null | undefined, ...args: FixedArray<Any>): int {
    if (delayMs) {
        return startTimer((): void => { func.unsafeCall(...args) }, delayMs, true);
    } else {
        return startTimer((): void => { func.unsafeCall(...args) }, 0, true);
    }
}

/**
 * Repeatedly print the error message with delayMs interval between calls. The first print will be after delayMs.
 *
 * @param { string } msg - the message to be printed.
 * @param { int | null | undefined}  delayMs - timeout in milliseconds, if pass empty or null or undefined will be treated as 0 milliseconds.
 * @returns { int } timer id
 */
export function setInterval(msg: string, delayMs?: int | null): int {
    if (delayMs) {
        return setInterval(():void => {
            console.error(msg)
        }, delayMs);
    } else {
        return setInterval(():void => {
            console.error(msg)
        }, 0);
    }
}

/**
 * Cancel the specified timer.
 *
 * @param { int | null | undefined} timerId - the id of the timer returned from setInterval, if pass empty or null or undefined will do nothing.
 */
export function clearInterval(timerId?: int | null): void {
    if (timerId) {
        stopTimer(timerId);
    }
}

function startTimer(func: () => void, delayMs: int, repeat: boolean): int
{
    const realDelayMs = delayMs;
    if (CoroutineExtras.isExternalTimerEnabled()) {
        return startTimerImpl(func as Object, realDelayMs < 0 ? 0 : realDelayMs, repeat);
    }
    return registerTimer(func, realDelayMs < 0 ? 0 : realDelayMs, repeat);
}

function stopTimer(timerId: int): void
{
    if (CoroutineExtras.isExternalTimerEnabled()) {
        stopTimerImpl(timerId);
        return;
    }
    clearTimer(timerId);
}

native function startTimerImpl(func: Object, delayMs: int, repeat: boolean): int;
native function stopTimerImpl(timerId: int): void;

native function registerTimer(func: Object, delayMs: int, repeat: boolean): int;
native function clearTimer(timerId: int): void;
