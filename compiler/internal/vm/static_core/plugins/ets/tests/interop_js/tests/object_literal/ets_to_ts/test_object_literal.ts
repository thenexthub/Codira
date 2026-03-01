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

const isEntity = etsVm.getFunction('Lobject_literal/ETSGLOBAL;', 'isEntity');
const Entity = etsVm.getClass('Lobject_literal/Entity;');
const isDataModel = etsVm.getFunction('Lobject_literal/ETSGLOBAL;', 'isDataModel');
const DataModel = etsVm.getClass('Lobject_literal/DataModel;');

const hasValidRecordProperty = etsVm.getFunction('Lobject_literal/ETSGLOBAL;', 'hasValidRecordProperty');
const testRecord = etsVm.getInstance('Lstd/core/Record;');

const IDataInterface = etsVm.getClass('Lobject_literal/object_literal$IDataInterface$ObjectLiteral;');
const isValidDataInterface = etsVm.getFunction('Lobject_literal/ETSGLOBAL;', 'isValidDataInterface');
const IUserProfile = etsVm.getClass('Lobject_literal/object_literal$IUserProfile$ObjectLiteral;');
const isValidUserProfile = etsVm.getFunction('Lobject_literal/ETSGLOBAL;', 'isValidUserProfile');

function main(): void {
    let testEntity;
    let isEntityResult = isEntity((testEntity = new Entity(), testEntity.id = 2, testEntity));
    ASSERT_TRUE(isEntityResult);

    let testData;
    let isDataResult = isDataModel((testData = new DataModel(), testData.numericId = 2,
                                    testData.stringCode = 'default', testData));
    ASSERT_TRUE(isDataResult);

    let isValidRecord = hasValidRecordProperty((testRecord['prop'] = 1, testRecord));
    ASSERT_TRUE(isValidRecord);

    try {
        etsVm.getInstance('Lescompat/wrongclass;');
    } catch (err) {
        ASSERT_EQ('Unsupported ETS instance type: Lescompat/wrongclass;', err.message);
    }

    let testDataInterface;
    let isInterfaceResult = isValidDataInterface((testDataInterface = new IDataInterface(),
                                                  testDataInterface.value = 1, testDataInterface));
    ASSERT_TRUE(isInterfaceResult);

    let testUserInterface;
    let isInterfaceResult1 = isValidUserProfile((testUserInterface = new IUserProfile(),
                                                 testUserInterface.name = 'Tom',
                                                 testUserInterface.age = 10,
                                                 testUserInterface));
    ASSERT_TRUE(isInterfaceResult1);
}

main();
