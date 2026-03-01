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
 * @Import { EtsConversions } from '../bu_d2s/libEtsConversions.ets';
 * @Tags interop, s2s
 */
class testToString {

    strChar: string = "A";
    strBin: string = "";
    strNumber: string = "";
    strHex: string = "";
    strOctal: string = "";

    generateNumber(seed: number): number {
        const modulus = Math.pow(2, 32);
        const a = 1664525;
        const c = 1013904223;
        seed = (a * seed + c) % modulus;
        return Math.floor((seed / modulus) * 100); 
    }

    /**
     * @Setup
     */
    public setup(): void {
        const seed: number = 123;
        const data: Number = this.generateNumber(seed);
        this.strBin = data.toString(2);
        this.strNumber = data.toString(10);
        this.strHex = data.toString(16);
        this.strOctal = data.toString(8);
    }

    /**
     * @Benchmark
     */
    bin(): string {
        return EtsConversions.binaryStringToNumberToString(this.strBin);
    }

    /**
     * @Benchmark
     */
    chr(): string {
        return EtsConversions.charToNumberToString(this.strChar);
    }

    /**
     * @Benchmark
     */
    decimal(): string {
        return EtsConversions.decimalStringToNumberToString(this.strNumber)
    }

    /**
     * @Benchmark
     */
    hex(): string {
        return EtsConversions.hexStringToNumberToString(this.strHex)
    }

    /**
     * @Benchmark
     */
    octal(): string {
        return EtsConversions.octalStringToNumberToString(this.strOctal);
    }

}
