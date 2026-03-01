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

package conversion_types.test;

const buffer: ArrayBuffer;
const dataView: DataView;
const globStsString: String;
const globStsNumber: Number;
const globStsInt: Int;
const globStsByte: Byte;
const globStsShort: Short;
const globStsLong: Long;
const globStsFloat: Float;
const globStsDouble: Double;
const globStsChar: Char;
const globStsBoolean: Boolean;
const globStsBigInt: BigInt;
static {
    buffer = new ArrayBuffer(1);
    dataView = new DataView(buffer);
    dataView.setUint8(0, 255);
    globStsString = new String("Hello");
    globStsNumber = new Number(42 as number);
    globStsInt = new Int(42);
    globStsByte = new Byte(126 as byte);
    globStsShort = new Short(4242 as short);
    globStsLong = new Long(42 as long);
    globStsFloat = new Float(42.42.toFloat());
    globStsDouble = new Double(42.42 as double);
    globStsChar = new Char(c'a');
    globStsBoolean = new Boolean(true);
    globStsBigInt = new BigInt(42);
}

class DeferentIntTypes {
    systemHexadecimal = 16;
    systemBinary = 2;
    num: int = 1;
    numb: number = 1;
    bigInt: BigInt = 1n;
    fl: float = 1.1f;
    intString: string = '1';
    l: long = 1;
    doub: double = 1;
    ch: char = c'1';
    by = dataView.getUint8(0);
}

class DeferentStringTypes {
    ch: char = c'a';
    str: string = "hello";
    lit: "one" | "two" = "one";
}

class NullAndUndefinedTypes {
    etsNull: null = null;
    etsUndefined: undefined = undefined;
}
