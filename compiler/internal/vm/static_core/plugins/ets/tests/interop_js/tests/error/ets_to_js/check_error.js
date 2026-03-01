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

const bar0 = stsVm.getFunction('Lerror/ETSGLOBAL;', 'bar0');
const bar1 = stsVm.getFunction('Lerror/ETSGLOBAL;', 'bar1');
const bar2 = stsVm.getFunction('Lerror/ETSGLOBAL;', 'bar2');

function catchErrorFromOrgJs() {
    try {
        bar0();
    } catch (e) {
        // print("e0: " + e)
        ASSERT_TRUE(e === 123);
    }

    try {
        bar1();
    } catch (e) {
        // print("e1: " + e)
        ASSERT_TRUE(e === 123);
    }

    try {
        bar2();
    } catch (e) {
        // print("e2: " + e.message)
        // print("e2 cause: " + e.cause.message)
        ASSERT_TRUE(e.message === 'current error');
        ASSERT_TRUE(e.cause.message === 'original error');
    }
}

catchErrorFromOrgJs();
