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
import {
    fnWithSubSetPartialParam,
    PartialTestUserType,
    TestUserTypeReduseProperty,
    fnWithSubSetReduseParam,
    TestUserType,
    fnWithUnionObjectParam,
    fnWithUnionParam,
    fnWithSubSetParam,
    BasicTestUser,
    fnWithAnyParams,
    fnWithExtraSetParam,
    fnWithLiteralObjectParam,
    ObjectTypeWithExtraSet,
    ObjectTypeWithLiteral,
    fnWithLiteralParam,
    fnWithAnyParamsObject,
    ObjectTypeWithAny
} from "js_import"

function optionalAnyParam(): boolean {
    return fnWithAnyParams('Param1', 'Param2') === 'Param1; Param2';
}

function optionalAnyWithoutParam(): boolean {
    return fnWithAnyParams('Param1') === 'Param1';
}

function optionalAnyObjectParam(): boolean {
    const testObj: ObjectTypeWithAny = {
        name: 'TestName',
        surname: 'TestSurname',
    }
    return fnWithAnyParamsObject(testObj) === 'TestName; TestSurname';
}

function optionalAnyObjectWithoutParam(): boolean {
    const testObj: ObjectTypeWithAny = {
        name: 'TestName'
    }
    return fnWithAnyParamsObject(testObj) === 'TestName';
}

function optionalLiteralParam(): boolean {
    return fnWithLiteralParam('TestName', 'Smith') === 'TestName; Smith';
}


function optionalLiteralWithoutParam(): boolean {
    return fnWithLiteralParam('TestName') === 'TestName';
}

function optionalLiteralObjectParam(): boolean {
    const testObj: ObjectTypeWithLiteral = {
        name: 'TestName',
        surname: 'Dou'
    }
    return fnWithLiteralObjectParam(testObj) === 'TestName; Dou';
}

function optionalLiteralObjectWithoutParam(): boolean {
    const testObj: ObjectTypeWithLiteral = {
        name: 'TestName'
    }
    return fnWithLiteralObjectParam(testObj) === 'TestName';
}

function optionalExtraSetParam(): boolean {
    return fnWithExtraSetParam('TestName', 'unknown string') === 'TestName; unknown string';
}

function optionalExtraSetWithoutParam(): boolean {
    return fnWithExtraSetParam('TestName') === 'TestName';
}

function optionalExtraSetObjectParam(): boolean {
    const testObj: ObjectTypeWithLiteral = {
        name: 'TestName',
        surname: 'unknown string'
    }
    return fnWithLiteralObjectParam(testObj) === 'TestName; unknown string'
}

function optionalExtraSetObjectWithoutParam(): boolean {
    const testObj: ObjectTypeWithLiteral = {
        name: 'TestName'
    }
    return fnWithLiteralObjectParam(testObj) === 'TestName'
}

function optionalSubSetObjectParam(): boolean {
    const testObj: BasicTestUser = {
        name: 'TestName',
        city: 'Tver'
    }

    return fnWithSubSetParam(testObj) === 'TestName; Tver'
}

function optionalSubSetObjectWithoutParam(): boolean {
    const testObj: BasicTestUser = {
        name: 'TestName'
    }

    return fnWithSubSetParam(testObj) === 'TestName'
}

function optionalSubSetObjectReduseParam(): boolean {
    const testObj: TestUserTypeReduseProperty = {
        name: 'TestName',
        surname: 'OptionalSurname'
    }

    return fnWithSubSetReduseParam(testObj) === 'TestName; OptionalSurname'
}

function optionalSubSetObjectPartialParam(): boolean {
    const testObj: PartialTestUserType = {
        name: 'TestName',
        surname: 'OptionalSecondSurname'
    }

    return fnWithSubSetPartialParam(testObj) === 'TestName; OptionalSecondSurname'
}

function optionalUnionStringParam(): boolean {

    return fnWithUnionParam('TestId') as string === 'TestId';
}

function optionalUnionStringWithoutParam(): boolean {

    return fnWithUnionParam() as string === 'id not found';
}

function optionalUnionNumberParam(): number {
    let res = fnWithUnionParam(999);
    return res as number;
}

function optionalUnionWithoutParam(): boolean {

    return fnWithUnionParam() as string === 'id not found';
}

function optionalUnionStringObjectParam(): boolean {
    const testObj: TestUserType = {
        name: 'TestName',
        id: 'TestId1'
    }

    return fnWithUnionObjectParam(testObj) === 'TestId1';
}

function optionalUnionNumberObjectParam(): number {
    const testObj: TestUserType = {
        name: 'TestName',
        id: 777
    }

    return fnWithUnionObjectParam(testObj) as number;
}

function optionalUnionObjectWithoutParam(): boolean {
    const testObj: TestUserType = {
        name: 'TestName'
    }

    return fnWithUnionObjectParam(testObj) === 'id not found';
}