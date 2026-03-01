/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import { returnMagic, returnMagicLambda } from "testDecl";
import { addTwo, addTwoLambda } from "testDecl";
import { addDefault, addDefaultLambda } from "testDecl";
import { addOptional, addOptionalLambda } from "testDecl";
import { addRest, addRestLambda } from "testDecl";

function testImportFunction() {
    arktest.assertEQ(returnMagic(), 0x55aa);
    arktest.assertEQ(returnMagicLambda(), 0x55aa);

    arktest.assertEQ(addTwo(1, 2), 3);
    arktest.assertEQ(addTwoLambda(1, 2), 3);

    arktest.assertEQ(addDefault(1, 2), 3);
    arktest.assertEQ(addDefaultLambda(1, 2), 3);
    arktest.assertEQ(addDefault(1), 1 + 0x7c00);
    arktest.assertEQ(addDefaultLambda(1), 1 + 0x7c00);

    arktest.assertEQ(addOptional(1, 2), 3);
    arktest.assertEQ(addOptionalLambda(1, 2), 3);
    arktest.assertEQ(addOptional(1), 1 + 0x7c00);
    arktest.assertEQ(addOptionalLambda(1), 1 + 0x7c00);

    arktest.assertEQ(addRest(1), 1);
    arktest.assertEQ(addRestLambda(1), 1);
    arktest.assertEQ(addRest(1, 2), 3);
    arktest.assertEQ(addRestLambda(1, 2), 3);
    arktest.assertEQ(addRest(1, 2, 3), 6);
    arktest.assertEQ(addRestLambda(1, 2, 3), 6);
    arktest.assertEQ(addRest(1, 2, 3, 4), 10);
    arktest.assertEQ(addRestLambda(1, 2, 3, 4), 10);
    arktest.assertEQ(addRest(1, 2, 3, 4, 5), 15);
    arktest.assertEQ(addRestLambda(1, 2, 3, 4, 5), 15);

    // NOTE(MockMockBlack): commented due to es2panda has bug here.
    // arktest.assertEQ(addRest(...[1, 2, 3, 4, 5, 6]), 21);
    // arktest.assertEQ(addRestLambda(...[1, 2, 3, 4, 5, 6]), 21);
}

function testFunctionAsArgument(getMagic: () => number) {
    arktest.assertEQ(getMagic(), 0x55aa);
}