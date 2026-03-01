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

import {util} from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("CommonUtil generateRandomBinaryUUID API tests");
    suite.addTest("Verify that a cached UUID is used.", testGenerateRandomBinaryUUIDWithCache);
    suite.addTest("Verify that a cached UUID is not used.", testGenerateRandomBinaryUUIDWithoutCache);
    return suite.run();
}

function testGenerateRandomBinaryUUIDWithCache() {
    const result = util.generateRandomBinaryUUID(true);
    arktest.assertEQ(result ? result!.length : 0, 16);
}

function testGenerateRandomBinaryUUIDWithoutCache() {
    const result = util.generateRandomBinaryUUID(false);
    arktest.assertEQ(result ? result!.length : 0, 16);
}
