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
 */

/**
 * @State
 * @Import { jsNumber,jsBigint,jsBoolean,jsNull,jsUndefined } from '../../shared/js/libValues.js'
 * @Import { jsString,jsObj,jsObjCopy,jsObjOther } from '../../shared/js/libValues.js'
 * @Include ../../shared/js/comparison.js
 * @Tags interop, d2d
 */
function equality() {

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareNumber = function() {
        return comparison(jsNumber, jsNumber, equal);
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.comparePrimitive = function() {
        let res = true;
        res &= comparison(jsBigint, jsBigint, equal);
        res &= comparison(jsBoolean, jsBoolean, equal);
        res &= comparison(jsNull, jsNull, equal);
        res &= comparison(jsUndefined, jsUndefined, equal);
        return res;
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareString = function() {
        return comparison(jsString, jsString, equal);
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareObject = function() {
        let res = true;
        res &= comparison(jsObj, jsObj, equal);
        res &= comparison(jsObj, jsObjCopy, equal);
        res &= comparison(jsObj, jsObjOther, noEqual);
        return res;
    };

}
