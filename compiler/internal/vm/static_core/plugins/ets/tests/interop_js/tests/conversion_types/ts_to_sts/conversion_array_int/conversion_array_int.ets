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

let global = ESValue.getGlobal();
let gtestEnv = global.getProperty("gtest_env");
let module = ESValue.load('main_js');

let tsInt = module.getProperty('tsInt');
let tsBigInt = module.getProperty('tsBigInt');
let tsBinary = module.getProperty('tsBinary');
let tsHexadecimal = module.getProperty('tsHexadecimal');
let tsIntString = module.getProperty('tsIntString');
let tsByte = module.getProperty('tsByte');
let tsFloat = module.getProperty('tsFloat');
let systemHexadecimal = module.getProperty('systemHexadecimal');
let systemBinary = module.getProperty('systemBinary');
let tsExponential = module.getProperty('tsExponential');

function checkConversionIntToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsInt);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsBinary, systemBinary);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionIntStringToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsIntString);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionFloatToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        val.setProperty(0, tsFloat);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionByteToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const byteValue = tsByte.invokeMethod('getUint8', ESValue.wrapNumber(0));
        val.setProperty(0, byteValue);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const NumberData = global.getProperty('Number');
        const converted = NumberData.invoke(tsBigInt);
        val.setProperty(0, converted);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayInt(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayNumber(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayFloat(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayByte(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayShort(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayLong(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayDouble(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionExponentialToArrayChar(): boolean {
    try {
        const val = ESValue.instantiateEmptyArray();
        const parseInt = global.getProperty('parseInt');
        const parsed = parseInt.invoke(tsExponential);
        val.setProperty(0, parsed);

        const firstElement = val.getProperty(0);
        return firstElement.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}
