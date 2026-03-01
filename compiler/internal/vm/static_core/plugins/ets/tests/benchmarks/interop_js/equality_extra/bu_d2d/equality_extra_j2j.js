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
 * @Import { jsArr,jsArrCopy,jsArrOther,jsArrowFoo } from '../../shared/js/libFunctions.js'
 * @Import { jsArrowFooCopy,jsArrowFooOther,jsFoo,jsFooOther } from '../../shared/js/libFunctions.js'
 * @Include ../../shared/js/comparison.js
 */

/**
 * @State
 * @Tags interop, d2d
 */
function equalityExtra() {

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareArrays = function() {
        let result = true;
        result &= comparison(jsArr, jsArr, equal);
        result &= comparison(jsArr, jsArrCopy, equal);
        result &= comparison(jsArr, jsArrOther, noEqual);
        return result;
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareArrowFuncs = function() {
        let result = true;
        result &= comparison(jsArrowFoo, jsArrowFoo, equal);
        result &= comparison(jsArrowFoo, jsArrowFooCopy, equal);
        result &= comparison(jsArrowFoo, jsArrowFooOther, noEqual);
        return result;
    };

    /**
     * @Benchmark
     * @returns {Bool}
     */
    this.compareFunc = function() {
        let result = true;
        result &= comparison(jsFoo, jsFoo, equal);
        result &= comparison(jsFoo, jsFooOther, noEqual);
        return result;
    };

}
