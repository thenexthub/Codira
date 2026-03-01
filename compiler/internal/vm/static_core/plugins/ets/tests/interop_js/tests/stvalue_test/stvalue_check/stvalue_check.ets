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

export class Student {
    constructor() { }
}

export class SubStudent extends Student {
    constructor() { super(); }
}

export namespace Check {
    export let subStu = new SubStudent();

    export let magicBoolean: boolean = false;
    export let magicByte: byte = 24;
    export let magicChar: char = c'h';
    export let magicShort: short = 124;
    export let magicInt: int = 15;
    export let magicLong: long = 102400;
    export let magicFloat: float = 3.14f
    export let magicDouble: double = 16.9999;
    export let magicString1: string = 'Hello World';
    export let magicString2: string = 'Hello World!';

    //isString isBigInt isBoolean isByte isChar isShort isInt isLong isFloat isNumber
    export let shouldBeString: string = "I am a string";
    export let treeString: String = 'I am a line string'.concat('you are a tree string');
    export let slicedString: String = 'I am a line string , you are a tree string'.substring(0, 15);
    export let shouldBeBigInt: bigint = 9007199254741991n;
    export let shouldBeBooleanTrue: boolean = true;
    export let shouldBeByte: byte = 72;
    export let shouldBeChar: char = c'a';
    export let shouldBeShort: short = 128;
    export let shouldBeInt: int = 42;
    export let shouldBeLong: long = 1024;
    export let shouldBeFloat: float = 3.14f;
    export let shouldBeNumber: double = 3.141592;

    // isNull isUndefined
    export let shouldBeNull = null;
    export let shouldBePrimitive: int = 123;
    export let shouldBeRef: Object = new Object();
    export let shouldBeUndefined = undefined;

    // isEqualTo()
    export let leftRef: string = 'isEqualTo';
    export let rightRef: string = 'isEqualTo';
    export let rightRefNotEqual: string = 'isEqualToNotEqual';
    export let rightRefNotSameType: Object = new Object();
    export let leftPrimitive: int = 42;
    export let rightPrimitive: int = 42;
    export let rightPrimitive2: int = 21;
    export let rightPrimitive3: short = 42;
    export let rightPrimitive4: long = 42;
}