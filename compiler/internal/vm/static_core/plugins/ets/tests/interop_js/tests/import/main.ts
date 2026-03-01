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

// Primitive types
export let tsNumber = 1;
export let tsString = 'Alice';
export let tsTrue = true;
export let tsFalse = false;
export let tsUndefined;
export let tsNull: null | number = null;
export let tsSymbol = Symbol('id');
export let tsNewSymbol: symbol = Symbol('new');
export let tsBigInt = 9007199254740991n;

export function increment(): number {
    return tsNumber++;
}

export function concatString(): string {
    return tsString += tsString;
}

export function changeTrue(): boolean {
    return tsTrue = !tsTrue;
}

export function changeFalse(): boolean {
    return tsFalse = !tsFalse;
}

export function changeUndefined(): number {
    return tsUndefined = tsNumber;
}

export function changeNull(): number {
    return tsNull = tsNumber;
}

export function changeSymbol(): Symbol {
    return tsSymbol = tsNewSymbol;
}

export function changeBigInt(): BigInt {
    return tsBigInt = tsBigInt - tsBigInt;
}

//Objects
export class TsClass {
    name = tsString;
}

export function changeTsClass(obj: TsClass): void {
    obj.name = tsString + tsString;
}

interface Iobj {
    value: string,
}
export const tsObject: Iobj = {
    value: 'tsString',
};

export function changeObject(): void {
    tsObject.value = tsString + tsString;
}

export const arr = [tsNumber];

export function changeArray(): void {
    arr.push(tsNumber);
}

export let tsFunction = (): number => tsNumber;

export function changeFunction(): void {
    tsFunction = (): number => tsNumber + tsNumber;
}

export let tsDate = new Date();

export function changeDate(): void {
    tsDate.setFullYear(tsDate.getFullYear() + 1);
};

export let tsReg = new RegExp('abc');

export function changeRegExp(): void {
    tsReg = new RegExp('xyz');
};

export default function defaultFu(): number {
    return tsNumber;
}

export class User {
    id: number;
}
