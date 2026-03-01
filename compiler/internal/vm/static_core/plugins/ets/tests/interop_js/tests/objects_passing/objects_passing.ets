/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

// Pass ArkTS object to JS (wrapping) (ArkTS calls TS)

let module = ESValue.load('js_import')
let updateObjectId = module.getProperty('updateObjectId');
let getOuterObj = module.getProperty('getOuterObj');
let testOuterObject = module.getProperty('testOuterObject');
let testSecondObjCls = module.getProperty('testSecondObjCls');
let testObjCls1 = module.getProperty('testObjCls1');
let testObjCls2 = module.getProperty('testObjCls2');
let testObject = module.getProperty('testObject');
let getName = module.getProperty('getName');
let getDetails = module.getProperty('getDetails');
let changeName = module.getProperty('changeName');
let getEdu = module.getProperty('getEdu');
let getObjectId = module.getProperty('getObjectId');
let getId = module.getProperty('getId');
let getObjectName = module.getProperty('getObjectName');

//Test 1 Get public property
function getPublicProperty(): string {
    return getName.invoke(testObjCls1, 'name').toString();
}

// Test 2 Use public method
function usePublicMethod(): string {
    return getDetails.invoke(testObjCls1).toString();
}

// Test 3 Change public propeerty
function changePublicProperty(): string {
    const newName = changeName.invoke(testObjCls2, 'NewTestName').toString();
    return newName;
}

// Test 4 Get readonly property 
function getReadonlyProperty(): string {
    return getEdu.invoke(testObjCls1).toString();
}

//Test 5 Get protected property
function getProtectedProperty(): number {
    return getObjectId.invoke(testSecondObjCls).toNumber();
}

//Test 6 Use protected method from subclass
function useProtectedMethod(): number {
    return getId.invoke(testSecondObjCls).toNumber();
}

//Test 7 get property from object
function getPropertyFromObject(): string {
    return getObjectName.invoke(testObject).toString();
}

// Test 8 Test with outer object
function getOuterObject(): number {
    return getOuterObj.invoke(testOuterObject).toNumber();
}

// Test 8 Update object value
function updateObjectValue(): number {
    updateObjectId.invoke(testObject, 333);
    return testObject.getProperty('id').toNumber();
}
