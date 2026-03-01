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

// Note: All tests return ets type `Object` i.e. `JSValue`


/**
 * @State
 * @Import {jsNumber,jsByte,jsShort,jsInt,jsLong,jsFloat,jsDouble} from '../../shared/js/libValues.js'
 * @Import {jsBigint,jsBoolean,jsNull,jsUndefined,jsString} from '../../shared/js/libValues.js'
 * @Tags interop, s2d
 */
class getValue {

    /**
     * @Benchmark
     */
    getNumber(): Object {
        let numVal = jsNumber;
        return numVal;
    };


    /**
     * @Benchmark
     */
    getByte(): Object {
        let byteVal = jsByte;
        return byteVal;
    };

    /**
     * @Benchmark
     */
    getShort(): Object {
        let shortVal = jsShort;
        return shortVal;
    };

    /**
     * @Benchmark
     */
    getInt(): Object {
        let intVal = jsInt;
        return intVal;
    };

    /**
     * @Benchmark
     */
    getLong(): Object {
        let longVal = jsLong;
        return longVal;
    };

    /**
     * @Benchmark
     */
    getFloat(): Object {
        let floatVal = jsFloat;
        return floatVal;
    };

    /**
     * @Benchmark
     */
    getDouble(): Object {
        let doubleVal = jsDouble;
        return doubleVal;
    };

    /**
     * @Benchmark
     */
    getBigInt(): Object {
        let bigIntVal = jsBigint;
        return bigIntVal;
    };

    /**
     * @Benchmark
     */
    getBool(): Object {
        let boolVal = jsBoolean;
        return boolVal;
    };

    /**
     * @Benchmark
     */
    getUndefined(): Object {
        let undefinedVal = jsUndefined;
        return undefinedVal;
    };

    /**
     * @Benchmark
     */
    getNull(): Object {
        let nullVal = jsNull;
        return nullVal;
    };

    /**
     * @Benchmark
     */
    getString(): Object {
        let strVal = jsString;
        return strVal;
    };

}
