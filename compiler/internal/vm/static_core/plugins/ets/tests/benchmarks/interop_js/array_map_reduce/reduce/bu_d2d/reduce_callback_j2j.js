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
 * @Import { sumFunction } from '../bu_s2d/libImport.ts'
 * @Tags interop, d2d
 */
function reduceCallback() {

    const min = 0;
    const max = 9999999;
    const arrayLength = 100;
    let testArray = [];
    let sumArray;

    function generateRandomArray(length, min, max, arr ) {
        for (let i = 0; i < length; i++) {
            const randomNumber = Math.floor(Math.random() * (max - min + 1)) + min;
            arr.push(randomNumber);
        }
    };

    /**
     * @Setup
     */
    this.setup = function () {
        generateRandomArray(arrayLength, min, max, testArray);
    };

    /**
     * @Benchmark
     * @returns {Int}
     */
    this.test = function() {
        sumArray = testArray.reduce(sumFunction, 0);
        return sumArray;
    };

}
