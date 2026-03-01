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

const stsVm = globalThis.gtest.etsVm;

const throwErrorFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwErrorFromSts');
const throwErrorWithCauseFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwErrorWithCauseFromSts');
const throwCustomErrorFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwCustomErrorFromSts');

function catchErrorFromSts() {
    try {
        throwErrorFromSts();
    } catch (e) {
        ASSERT_TRUE(e.name === 'Error');
        ASSERT_TRUE(e.message === 'Throw Error from ets!');
    }
}

function catchErrorWithCauseFromSts() {
    try {
        throwErrorWithCauseFromSts();
    } catch (e) {
        ASSERT_TRUE(e.name === 'Error');
        ASSERT_TRUE(e.message === 'Throw Error with cause from ets!');
        ASSERT_TRUE(e.cause === 'Test cause');
    }
}

function catchCustomErrorFromSts() {
    try {
        throwCustomErrorFromSts();
    } catch (e) {
        ASSERT_TRUE(e.name === 'CustomError');
        ASSERT_TRUE(e.message === 'Throw Custom Error from ets!');
        ASSERT_TRUE(e.code === 12345);
    }
}

catchErrorFromSts();
catchErrorWithCauseFromSts();
catchCustomErrorFromSts();
