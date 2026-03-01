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
    fnWithAnyParamObject,
    ObjectTypeWithArr,
    TestUserType,
    fnWithUserInterface,
    fnWithUserClass,
    TestUserClass,
    UnionArrOrObj,
    fnWithUnionParam,
    UserPartial,
    UserOmit,
    fnWithSubsetPick,
    UserPick,
    AddressType,
    fnWithExtraSetParam,
    fnWithLiteralParam,
    fnWithAnyParams
} from "js_code"

function referenceAnyParam(): boolean {
    return fnWithAnyParams(['one', 'two']) === 'one';
}

function referenceAnyWithoutParam(): boolean {
    return fnWithAnyParams() === 'Argument not found'
}

function referenceLiteralParam(): boolean {
    return fnWithLiteralParam(['Moscow', 'London', 'Paris']) === 'Moscow';
}

function referenceLiteralWithoutParam(): boolean {
    return fnWithLiteralParam() === 'Argument not found';
}

function referenceExtraSetParam(): boolean {
    return fnWithExtraSetParam(['Param1', 'Param2', 'Param3']) === 'Param1';
}

function referenceExtraSetWithoutParam(): boolean {
    return fnWithExtraSetParam() as string === 'Argument not found';
}

function referenceSubsetPickParam(): boolean {
    const testAddress: AddressType = {
        street: 'main',
        city: 'Moscow'
    };
    const testObj: UserPick = {
        name: 'Fill',
        address: testAddress
    };
    return fnWithSubsetPick(testObj) === 'Moscow';
}

function referenceSubsetPickWithoutParam(): boolean {
    const testObj: UserPick = {
        name: 'Fill'
    };
    return fnWithSubsetPick(testObj) === 'Address not found';
}

function referenceSubsetOmitParam(): boolean {
    const testAddress: AddressType = {
        street: 'main',
        city: 'London'
    };
    const testObj: UserPick = {
        name: 'Fill',
        address: testAddress
    };
    return fnWithSubsetPick(testObj) === 'London';
}

function referenceSubsetOmitWithoutParam(): boolean {
    const testObj: UserPick = {
        name: 'Fill'
    };
    return fnWithSubsetPick(testObj) === 'Address not found';
}

function referenceSubsetPartialParam(): boolean {
    const testAddress: AddressType = {
        street: 'main',
        city: 'London'
    };
    const testObj: UserPick = {
        name: 'Fill',
        address: testAddress
    };
    return fnWithSubsetPick(testObj) === 'London';
}

function referenceSubsetPartialWithoutParam(): boolean {
    const testObj: UserPick = {
        name: 'Fill'
    };
    return fnWithSubsetPick(testObj) === 'Address not found';
}

function referenceUnionArrParam(): boolean {
    const arr: FixedArray<string> = ['param1', 'param2'];
    return fnWithUnionParam(arr) === 'This is an array';
}

interface IUnionArr {
    name: string,
    id: number,
}
function referenceUnionObjParam(): boolean {
    const testObj: IUnionArr = {
        name: 'testName',
        id: 123
    };
    return fnWithUnionParam(testObj) === 'This is an object';
}

function referenceUnionWithoutParam(): boolean {
    return fnWithUnionParam() === 'Argument not found';
}

function referenceClassParam(): boolean {
    const testUser = new TestUserClass(123, 'TestName');
    return fnWithUserClass(testUser) === 'TestName';
}

function referenceClassWithoutParam(): boolean {
    return fnWithUserClass() === 'Class was not passed';
}

function referenceInterfaceParam(): boolean {
    const testObj: TestUserType = {
        id: 321,
        name: 'TestName1',
        age: 30
    };
    return fnWithUserInterface(testObj) === 'TestName1'
}

function referenceInterfaceWithoutParam(): boolean {
    return fnWithUserInterface() === 'Object was not passed';
}

// NOTE(pishin) enable after fixing #17878
//  function referenceObjectWithArr(): boolean {
//    const testObj:ObjectTypeWithArr = {
//       id: 123,
//       arr: ['param', 'param2']
//    };
//    return fnWithAnyParamObject(testObj) == 'param';
//  }