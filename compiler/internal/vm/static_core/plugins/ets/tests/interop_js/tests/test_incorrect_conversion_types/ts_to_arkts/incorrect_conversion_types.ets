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

import {
    testInterface,
    tsString,
    tsNumber,
    tsNumberToByteLR,
    tsNumberToByteRR,
    tsNumberToIntLR,
    tsNumberToIntRR,
    tsNumberToShortLR,
    tsNumberToShortRR,
    tsNumberToLongLR,
    tsNumberToLongRR,
    tsNumberToCharLR,
    tsNumberToCharRR,
    tsBoolean,
    tsNull,
    tsUndefined,
    tsBigInt,
    tsArrayNum,
    tsArrayStr
} from 'js_code';

function isTypeError(error: Error): boolean{
    const errorStr: string = '' + error.cause;
    if (errorStr.toLowerCase().includes('typeerror')) {
        return true;
    }
    return false;
}

function falseIfThrows(fn: () => void): boolean {
    try {
        fn();
        console.log('falseIfThrows');
        return false;
    } catch (error) {
        return isTypeError(error as Error);
    }
}

function conversionTsObjToStr(): boolean { return falseIfThrows(() => { testInterface as string } ) }
function conversionTsArrayNumToStr(): boolean { return falseIfThrows(() => { tsArrayNum as string } ) }
function conversionTsArrayStrToStr(): boolean { return falseIfThrows(() => { tsArrayStr as string } ) }
function conversionTsArrayNumToNum(): boolean { return falseIfThrows(() => { tsArrayNum as number } ) }
function conversionTsStrToBool(): boolean { return falseIfThrows(() => { tsString as boolean } ) }
function conversionTsStrToNum(): boolean { return falseIfThrows(() => { tsString as number } ) }
function conversionTsStrToInt(): boolean { return falseIfThrows(() => { tsString as int } ) }
function conversionTsStrToByte(): boolean { return falseIfThrows(() => { tsString as byte } ) }
function conversionTsStrToShort(): boolean { return falseIfThrows(() => { tsString as short } ) }
function conversionTsStrToLong(): boolean { return falseIfThrows(() => { tsString as long } ) }
function conversionTsStrToFloat(): boolean { return falseIfThrows(() => { tsString as float } ) }
function conversionTsStrToDouble(): boolean { return falseIfThrows(() => { tsString as double } ) }
function conversionTsStrToChar(): boolean { return falseIfThrows(() => { tsString as char } ) }
function conversionTsStrToBigInt(): boolean { return falseIfThrows(() => { tsString as bigint } ) }
function conversionTsBooleanToNum(): boolean { return falseIfThrows(() => { tsBoolean as number } ) }
function conversionTsBooleanToByte(): boolean { return falseIfThrows(() => { tsBoolean as byte } ) }
function conversionTsBooleanToShort(): boolean { return falseIfThrows(() => { tsBoolean as short } ) }
function conversionTsBooleanToInt(): boolean { return falseIfThrows(() => { tsBoolean as int } ) }
function conversionTsBooleanToLong(): boolean { return falseIfThrows(() => { tsBoolean as long } ) }
function conversionTsBooleanToFloat(): boolean { return falseIfThrows(() => { tsBoolean as float } ) }
function conversionTsBooleanToDouble(): boolean { return falseIfThrows(() => { tsBoolean as double } ) }
function conversionTsBooleanToChar(): boolean { return falseIfThrows(() => { tsBoolean as char } ) }
function conversionTsBooleanToStr(): boolean { return falseIfThrows(() => { tsBoolean as string } ) }
function conversionTsBooleanToBigInt(): boolean { return falseIfThrows(() => { tsBoolean as boolean } ) }
function conversionTsUndefinedToNum(): boolean { return falseIfThrows(() => { tsUndefined as number } ) }
function conversionTsUndefinedToByte(): boolean { return falseIfThrows(() => { tsUndefined as byte } ) }
function conversionTsUndefinedToShort(): boolean { return falseIfThrows(() => { tsUndefined as short } ) }
function conversionTsUndefinedToInt(): boolean { return falseIfThrows(() => { tsUndefined as int } ) }
function conversionTsUndefinedToLong(): boolean { return falseIfThrows(() => { tsUndefined as long } ) }
function conversionTsUndefinedToFloat(): boolean { return falseIfThrows(() => { tsUndefined as float } ) }
function conversionTsUndefinedToDouble(): boolean { return falseIfThrows(() => { tsUndefined as double } ) }
function conversionTsUndefinedToChar(): boolean { return falseIfThrows(() => { tsUndefined as char } ) }
function conversionTsUndefinedToBool(): boolean { return falseIfThrows(() => { tsUndefined as boolean } ) }
function conversionTsUndefinedToStr(): boolean { return falseIfThrows(() => { tsUndefined as string } ) }
function conversionTsUndefinedToBigInt(): boolean { return falseIfThrows(() => { tsUndefined as boolean } ) }
function conversionTsNullToNum(): boolean { return falseIfThrows(() => { tsNull as number } ) }
function conversionTsNullToByte(): boolean { return falseIfThrows(() => { tsNull as byte } ) }
function conversionTsNullToShort(): boolean { return falseIfThrows(() => { tsNull as short } ) }
function conversionTsNullToInt(): boolean { return falseIfThrows(() => { tsNull as int } ) }
function conversionTsNullToLong(): boolean { return falseIfThrows(() => { tsNull as long } ) }
function conversionTsNullToFloat(): boolean { return falseIfThrows(() => { tsNull as float } ) }
function conversionTsNullToDouble(): boolean { return falseIfThrows(() => { tsNull as double } ) }
function conversionTsNullToChar(): boolean { return falseIfThrows(() => { tsNull as char } ) }
function conversionTsNullToBool(): boolean { return falseIfThrows(() => { tsNull as boolean } ) }
function conversionTsNullToBigInt(): boolean { return falseIfThrows(() => { tsNull as bigint } ) }
function conversionTsBigIntToNum(): boolean { return falseIfThrows(() => { tsBigInt as number } ) }
function conversionTsBigIntToByte(): boolean { return falseIfThrows(() => { tsBigInt as byte } ) }
function conversionTsBigIntToShort(): boolean { return falseIfThrows(() => { tsBigInt as short } ) }
function conversionTsBigIntToInt(): boolean { return falseIfThrows(() => { tsBigInt as int } ) }
function conversionTsBigIntToLong(): boolean { return falseIfThrows(() => { tsBigInt as long } ) }
function conversionTsBigIntToFloat(): boolean { return falseIfThrows(() => { tsBigInt as float } ) }
function conversionTsBigIntToDouble(): boolean { return falseIfThrows(() => { tsBigInt as double } ) }
function conversionTsBigIntToChar(): boolean { return falseIfThrows(() => { tsBigInt as char } ) }
function conversionTsBigIntToBool(): boolean { return falseIfThrows(() => { tsBigInt as boolean } ) }
function conversionTsBigIntToStr(): boolean { return falseIfThrows(() => { tsBigInt as string } ) }
function conversionTsNumToBool(): boolean { return falseIfThrows(() => { tsNumber as boolean } ) }
function conversionTsNumToStr(): boolean { return falseIfThrows(() => { tsNumber as string } ) }
function conversionTsNumToBigInt(): boolean { return falseIfThrows(() => { tsNumber as bigint } ) }

function conversionTsNumToInt(): boolean {
    try {
        const expValLR: int = -2147483648;
        const expValRR: int = 2147483647;

        const testValLR = tsNumberToIntLR as number;
        const testValRR = tsNumberToIntRR as number;

        return ((expValLR == testValLR) && (expValRR == testValRR));
    } catch (error) {
        return isTypeError(error as Error);
    }
}

function conversionTsNumToByte(): boolean {
    try {
        const expValLR: byte = 127;
        const expValRR: byte = -128;

        const testValLR = tsNumberToByteLR as byte;
        const testValRR = tsNumberToByteRR as byte;

        return ((expValLR == testValLR) && (expValRR == testValRR));
    } catch (error) {
        return isTypeError(error as Error);
    }
}

function conversionTsNumToShort(): boolean {
    try {
        const expValLR: short = 32767;
        const expValRR: short = -32768;

        const testValLR = tsNumberToShortLR as short;
        const testValRR = tsNumberToShortRR as short;

        return ((expValLR == testValLR) && (expValRR == testValRR));
    } catch (error) {
        return isTypeError(error as Error);
    }
}

function conversionTsNumToLong(): boolean {
    try {
        const expValLR: long = -9223372036854775808;
        const expValRR: long = 9223372036854775807;

        const testValLR = tsNumberToLongLR as long;
        const testValRR = tsNumberToLongRR as long;

        return ((expValLR == testValLR) && (expValRR == testValRR));
    } catch (error) {
        return isTypeError(error as Error);
    }
}

function conversionTsNumToChar(): boolean {
    try {
        const expValLR: char = c'\uF8FF';
        const expValRR: char = c'\u0100';

        const testValLR = tsNumberToCharLR as char;
        const testValRR = tsNumberToCharRR as char;

        return ((expValLR == testValLR) && (expValRR == testValRR));
    } catch (error) {
        return isTypeError(error as Error);
    }
}
