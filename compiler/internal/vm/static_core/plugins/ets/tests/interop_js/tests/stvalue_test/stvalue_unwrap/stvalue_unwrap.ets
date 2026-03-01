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

export namespace Unwrap {
    export let magicBoolean: boolean = false;
    export let magicByte: byte = 24;
    export let magicChar: char = c'h';
    export let magicShort: short = 124;
    export let magicInt: int = 15;
    export let magicLong: long = 102400;
    export let magicFloat: float = 3.14f
    export let magicDouble: double = 3.1415926
    export let magicString1: string = 'Hello World';
    export let magicString2: string = 'Hello World!!!';

    /*
    * toBigIntTest
    */
    export let defaultNumber: int = 42;
    export let bigIntFromString: bigint = BigInt('999999999999999999');
    export let bigIntFromLiteral: bigint = 123456789012345678901234567890n;
    export let stringVariable: string = 'Hello, World!';
    export let zeroBigInt: bigint = 0n;
    export let negativeBigInt: bigint = -1234567890123456789n;
}