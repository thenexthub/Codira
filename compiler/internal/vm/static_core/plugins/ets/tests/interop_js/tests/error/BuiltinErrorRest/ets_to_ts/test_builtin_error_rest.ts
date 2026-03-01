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

const etsVm = globalThis.gtest.etsVm;

const linkerClassNotFoundError = etsVm.getClass('Lbuiltin_error_rest/ETSGLOBAL;').linkerClassNotFoundError;
const throwLinkerClassNotFoundError = etsVm.getFunction('Lbuiltin_error_rest/ETSGLOBAL;', 'throwLinkerClassNotFoundError');

function testGetError() {
    ASSERT_TRUE(linkerClassNotFoundError.toString() === 'LinkerClassNotFoundError: LinkerClassNotFoundError from ETS');
}

function testThrowError() {
    let res: false;
    let msg = 'throw Error from ETS!';
    try {
        throwLinkerClassNotFoundError(msg);
    } catch(err) {
        res = (err.toString() === ("LinkerClassNotFoundError: " + msg));
    }
    ASSERT_TRUE(res);
}

testGetError();
testThrowError();
