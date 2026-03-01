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

let tsHexadecimal = module.getProperty('tsHexadecimal');
let systemHexadecimal = module.getProperty('systemHexadecimal');

function checkConversionHexadecimalToInt(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToNumber(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToFloat(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToByte(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToShort(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToLong(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToDouble(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionHexadecimalToChar(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsHexadecimal, systemHexadecimal);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}
