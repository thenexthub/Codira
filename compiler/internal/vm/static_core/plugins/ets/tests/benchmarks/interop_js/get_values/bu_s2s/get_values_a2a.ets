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

/**
 * @State
 * @Import { ValuesHolder } from '../bu_d2s/libValuesHolder.ets'
 * @Tags interop, s2s
 */
class getValue {

    /**
     * @Benchmark
     */
    getNumber(): number {
        let numVal = ValuesHolder.etsNumber;
        return numVal;
    };


    /**
     * @Benchmark
     */
    getByte(): byte {
        let byteVal = ValuesHolder.etsByte;
        return byteVal;
    };

    /**
     * @Benchmark
     */
    getShort(): short {
        let shortVal = ValuesHolder.etsShort;
        return shortVal;
    };

    /**
     * @Benchmark
     */
    getInt(): int {
        let intVal = ValuesHolder.etsInt;
        return intVal;
    };

    /**
     * @Benchmark
     */
    getLong(): long {
        let longVal = ValuesHolder.etsLong;
        return longVal;
    };

    /**
     * @Benchmark
     */
    getFloat(): float {
        let floatVal = ValuesHolder.etsFloat;
        return floatVal;
    };

    /**
     * @Benchmark
     */
    getDouble(): double {
        let doubleVal = ValuesHolder.etsDouble;
        return doubleVal;
    };

    /**
     * @Benchmark
     */
    getBigInt(): bigint {
        let bigIntVal = ValuesHolder.etsBigint;
        return bigIntVal;
    };

    /**
     * @Benchmark
     */
    getBool(): boolean {
        let boolVal = ValuesHolder.etsBoolean;
        return boolVal;
    };

    /**
     * @Benchmark
     */
    getUndefined(): undefined {
        let undefinedVal = ValuesHolder.etsUndefined;
        return undefinedVal;
    };

    /**
     * @Benchmark
     */
    getNull(): null {
        let nullVal = ValuesHolder.etsNull;
        return nullVal;
    };

    /**
     * @Benchmark
     */
    getString(): string {
        let strVal = ValuesHolder.etsString;
        return strVal;
    };


}
