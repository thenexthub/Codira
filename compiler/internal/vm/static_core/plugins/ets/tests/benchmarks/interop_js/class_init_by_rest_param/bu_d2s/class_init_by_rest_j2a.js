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
function classInitByRest() {

    /**
     * @Param 5, 200
     */
    this.size;
    this.arr;
    this.UnlimitedArguments = null;

    this.createArgArray = function(count) {
        this.arr = [];
        for (let i = 0; i < count; i++) {
            this.arr.push(i);
        }
    };

    /**
     * @Setup
     */
    this.setup = function() {
        let stsVm = initEtsVm();
        this.UnlimitedArguments = stsVm.getClass('LlibClassVarArg/UnlimitedArguments;');
        this.createArgArray(this.size);
    };

    /**
     * @Benchmark
     */
    this.test = function() {
        // ClassCastError: $std.core.Array$ cannot be cast to std.core.Double
        new this.UnlimitedArguments(this.arr);
    };

}
