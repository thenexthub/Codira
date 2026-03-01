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

let tsBigInt = module.getProperty('tsBigInt');
let returnBigInt = module.getProperty('returnBigInt');
let returnBigIntObj = module.getProperty('returnBigIntObj');

function checkTypeBigInt(): boolean {
    try {
        return tsBigInt.typeOf() == 'bigint';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToInt(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToNumber(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToFloat(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToByte(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToShort(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToLong(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToDouble(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBigIntToChar(): boolean {
    try {
        const NumberData = global.getProperty('Number');
        const val = NumberData.invoke(tsBigInt);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkReturnBigInt(): boolean {
    try {
        const val = returnBigInt.invoke();

        if (!val.isBigInt()) {
            return false;
        }

        const expectedBigInt = ESValue.wrapBigInt(1000000n);
        return val.toBigInt() == expectedBigInt.toBigInt();
    } catch (e) {
        return false;
    }
}

function checkReturnBigIntObj(): boolean {
    try {
        const val = returnBigIntObj.invoke();

        if (!val.isBigInt()) {
            return false;
        }

        const expectedBigInt = ESValue.wrapBigInt(1n);
        return val.toBigInt() == expectedBigInt.toBigInt();
    } catch (e) {
        return false;
    }
}