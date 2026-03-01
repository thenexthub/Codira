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
function callImportFunction() {

    this.etsVoid = null;
    this.anonymous = null;

    /**
      * @Setup
      */
    this.setup = function () {
        let stsVm = initEtsVm();
        this.etsVoid = stsVm.getFunction('LcallImportFunction/ETSGLOBAL;', 'stsVoid');
        let returnAnonymous = stsVm.getFunction('LcallImportFunction/ETSGLOBAL;', 'returnAnonymous');
        this.anonymous = returnAnonymous();
    };

    function callFunction(fun) {
        return fun(1000000, 1000000);
    };

    /**
     * @Benchmark
     */
    this.callFun = function() {
        // InteropCtx::Fatal: NAPI_CHECK_FATAL: napi_typeof(env, val, &vtype) status=1
        callFunction(this.etsVoid);
    };

    /**
     * @Benchmark
     */
    this.callAnonym = function() {
        // InteropCtx::Fatal: NAPI_CHECK_FATAL: napi_typeof(env, val, &vtype) status=1
        callFunction(this.anonymous);
    };

}


