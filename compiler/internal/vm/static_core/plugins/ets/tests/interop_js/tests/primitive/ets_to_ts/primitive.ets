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


function getBooleanPrimitive(): boolean {
    return true
}

function getBooleanBoxed(): Boolean {
    return new Boolean(true)
}

function getBytePrimitive(): byte {
    return 1
}

function getByteBoxed(): Byte {
    return new Byte(1 as byte)
}

function getShortPrimitive(): short {
    return 1
}

function getShortBoxed(): Short {
    return new Short(1 as short)
}

function getIntPrimitive(): int {
    return 1
}

function getIntBoxed(): Int {
    return new Int(1)
}

function getLongPrimitive(): long {
    return 1
}

function getLongBoxed(): Long {
    return new Long(1)
}

function getFloatPrimitive(): float {
    return 1.0f
}

function getFloatBoxed(): Float {
    return new Float(1)
}

function getNumberPrimitive(): number {
    return 1.0
}

function getNumberBoxed(): Number {
    return new Number(1)
}

function getDoublePrimitive(): double {
    return 1.0
}

function getDoubleBoxed(): Double {
    return new Double(1)
}

function getStringPrimitive(): string {
    return '1'
}

function getStringBoxed(): String {
    return new String('1')
}

function getCharPrimitive(): char {
    return c'1'
}

function getCharPrimitiveNumber(): char {
    return Int.toChar(49)
}

function getCharBoxed(): Char {
    return new Char(c'1')
}

function getCharBoxedNumber(): Char {
    return new Char(Int.toChar(49))
}


function getBigIntPrimitive(): bigint {
    return 1n
}

function getBigIntBoxed(): BigInt {
    return new BigInt(1n)
}

function getNullPrimitive() {
    return null
}

function getUndefinedPrimitive() {
    return undefined
}
