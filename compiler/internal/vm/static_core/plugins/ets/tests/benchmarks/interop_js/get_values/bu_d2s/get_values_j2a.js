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
 * @Include ../../shared/js/initEtsVm.js
 * @Tags interop, d2s
 */
function getValue() {

    this.holder = null;

    /**
     * @Setup
     */
    this.setup = function () {
        let stsVm = initEtsVm();
        this.holder = stsVm.getClass('LlibValuesHolder/ValuesHolder;');
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.getNumber = function() {
        let numVal = this.holder.etsNumber;
        return numVal;
    };


    /**
     * @Benchmark
     * @returns {Char}
     */
    this.getByte = function() {
        let byteVal = this.holder.etsByte;
        return byteVal;
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.getShort = function() {
        let shortVal = this.holder.etsShort;
        return shortVal;
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.getInt = function() {
        let intVal = this.holder.etsInt;
        return intVal;
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.getLong = function() {
        let longVal = this.holder.etsLong;
        return longVal;
    };

    /**
     * @Benchmark
     * @returns {Float}
     */
    this.getFloat = function() {
        let floatVal = this.holder.etsFloat;
        return floatVal;
    };

    /**
     * @Benchmark
     * @returns {Float}
     */
    this.getDouble = function() {
        let doubleVal = this.holder.etsDouble;
        return doubleVal;
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.getBigInt = function() {
        let bigIntVal = this.holder.etsBigint;
        return bigIntVal;
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.getBool = function() {
        let boolVal = this.holder.etsBoolean;
        return boolVal;
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.getUndefined = function() {
        let undefinedVal = this.holder.etsUndefined;
        return undefinedVal;
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.getNull = function() {
        let nullVal = this.holder.etsNull;
        return nullVal;
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.getString = function() {
        let strVal = this.holder.etsString;
        return strVal;
    };

}
