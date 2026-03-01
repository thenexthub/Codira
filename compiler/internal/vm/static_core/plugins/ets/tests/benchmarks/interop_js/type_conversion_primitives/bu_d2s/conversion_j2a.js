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
 * @Include ../bu_d2d/generateNumber.js
 * @Tags interop, d2s
 */
function testToString() {

    this.ets = null;
    this.strChar = 'A';
    this.strBin = '';
    this.strNumber = '';
    this.strHex = '';
    this.strOctal = '';

    /**
     * @Setup
     */
    this.setup = function () {
        const seed = 123;
        const data = generateNumber(seed);
        this.strBin = data.toString(2);
        this.strNumber = data.toString(10);
        this.strHex = data.toString(16);
        this.strOctal = data.toString(8);
        let stsVm = initEtsVm();
        this.ets = stsVm.getClass('LlibEtsConversions/EtsConversions;');
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.bin = function() {
        return this.ets.binaryStringToNumberToString(this.strBin);
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.decimal = function() {
        return this.ets.decimalStringToNumberToString(this.strNumber);
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.hex = function() {
        return this.ets.hexStringToNumberToString(this.strHex);
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.octal = function() {
        return this.ets.octalStringToNumberToString(this.strOctal);
    };

    /**
     * @Benchmark
     * @returns {Obj}
     */
    this.chr = function() {
        return this.ets.charToNumberToString(this.strChar);
    };

}
