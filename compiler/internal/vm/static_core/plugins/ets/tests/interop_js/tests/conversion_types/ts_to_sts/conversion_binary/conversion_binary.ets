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

let tsBinary = module.getProperty('tsBinary');
let systemBinary = module.getProperty('systemBinary');

function checkConversionBinaryToInt(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToNumber(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToFloat(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToByte(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToShort(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToLong(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToDouble(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionBinaryToChar(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsBinary, systemBinary);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}
