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
 * @Tags interop, d2s
 * @Include ../../shared/js/initEtsVm.js
 * @Include ../../shared/js/comparison.js
 */
function equality() {

    this.bench = null;

    /**
     * @Setup
     */
    this.setup = function () {
        let stsVm = initEtsVm();
        this.ets = stsVm.getClass('LlibValues/Values;');
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareNumber = function() {
        return comparison(this.ets.etsNum, this.ets.etsNum, equal);
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.comparePrimitive = function() {
        let res = true;
        res &= comparison(this.ets.etsBigint, this.ets.etsBigint, equal);
        res &= comparison(this.ets.etsBoolean, this.ets.etsBoolean, equal);
        res &= comparison(this.ets.etsNull, this.ets.etsNull, equal);
        res &= comparison(this.ets.etsUndefined, this.ets.etsUndefined, equal);
        return res;
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareString = function() {
        return comparison(this.ets.etsString, this.ets.etsString, equal);
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareObject = function() {
        let res = true;
        res &= comparison(this.ets.etsObj, this.ets.etsObj, equal);
        res &= comparison(this.ets.etsObj, this.ets.etsObjCopy, equal);
        res &= comparison(this.ets.etsObj, this.ets.etsObjOther, noEqual);
        return res;
    };

}
