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

package interopStandaloneFnTest

const functionRetTypeNumberEts: () => number;

const functionRetTypeByteEts: () => byte;

const functionRetTypeShortEts: () => short;

const functionRetTypeIntEts: () => int;

const functionRetTypeLongEts: () => long;

const functionRetTypeFloatEts: () => float;

const functionRetTypeCharEts: () => char;

const functionRetTypeBooleanEts: () => boolean;

const functionRetTypeStringEts: () => string;

function functionRetTypeNumberLitEts(): number {
    return 1;
}

function functionRetTypeByteLitEts(): byte {
    return 1;
}

function functionRetTypeShortLitEts(): Short {
    return 1;
}

function functionRetTypeIntLitEts(): int {
    return 1;
}

function functionRetTypeLongLitEts(): long {
    return 1;
}

function functionRetTypeFloatLitEts(): float {
    return 1.0;
}

function functionRetTypeCharLitEts(): Char {
    return c'c';
}

function functionRetTypeBooleanLitEts(): Boolean {
    return true;
}

function functionRetTypeStringLitEts(): string {
    return 'test';
}

class X {
    public id: number = 0;
}

function functionRetTypeObjectEts(): X {
    let x: X = {id: 1};
    return x;
}

function functionRetTypeClassEts(): ClassWithMethodEts {
    let x: ClassWithMethodEts = new ClassWithMethodEts();
    return x;
}

function functionRetTypeArrayEts(): FixedArray<string> {
    let x: FixedArray<string> = ['one', 'two'];
    return x;
}

function functionRetTypeTupleEts(): [number, string] {
    let x: [number, string] = [1, 'one'];
    return x;
}

function functionRetTypeCallableEts(): () => int {
    return standaloneFunctionEts();
}

function functionRetTypeInterfaceEts(): ImplInterfaceWithMethodEts {
    return new ImplInterfaceWithMethodEts();
}
