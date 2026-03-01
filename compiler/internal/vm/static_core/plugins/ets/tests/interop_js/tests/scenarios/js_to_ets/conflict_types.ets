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

let module = ESValue.load('pure_js_conflict');
let functionArgTypeConflictArray = module.getProperty('functionArgTypeConflictArray');
let functionArgTypeConflictArrayBuffer = module.getProperty('functionArgTypeConflictArrayBuffer');
let functionArgTypeConflictBoolean = module.getProperty('functionArgTypeConflictBoolean');
let functionArgTypeConflictDataView = module.getProperty('functionArgTypeConflictDataView');
let functionArgTypeConflictDate = module.getProperty('functionArgTypeConflictDate');
let functionArgTypeConflictError = module.getProperty('functionArgTypeConflictError');
let functionArgTypeConflictMap = module.getProperty('functionArgTypeConflictMap');
let functionArgTypeConflictObject = module.getProperty('functionArgTypeConflictObject');
let functionArgTypeConflictString = module.getProperty('functionArgTypeConflictString');

function TestFunctionArgTypeConflictArray(): boolean {
    const array:Array<Object> = functionArgTypeConflictArray.invoke().unwrap() as Array<Object>;
    const EXPECTED_LENGTH = 2;
    const EXPECTED_INT = 123;
    const EXPECTED_STRING = 'foo';

    arktest.assertTrue(array.length === EXPECTED_LENGTH);

    return (array.at(0) as Number === EXPECTED_INT) && (array.at(1) as string === EXPECTED_STRING);
}

function TestFunctionArgTypeConflictArraybuffer(): boolean {
    const buffer = InteropTransferHelper.transferArrayBufferToStatic(functionArgTypeConflictArrayBuffer.invoke().unwrap());
    const view = new Int32Array(buffer as ArrayBuffer);
    const EXPECTED_INT = 123;

    return view.at(0) === EXPECTED_INT;
}

function TestFunctionArgTypeConflictBoolean(): boolean {
    const value = functionArgTypeConflictBoolean.invoke().unwrap() as Boolean;
    return value === true;
}

function TestFunctionArgTypeConflictDataview(): boolean {
    const view = functionArgTypeConflictDataView.invoke().unwrap() as DataView;
    const EXPECTED_INT = 123;

    return view.getInt32(0) === EXPECTED_INT;
}

function TestFunctionArgTypeConflictDate(): boolean {
    const date = functionArgTypeConflictDate.invoke().unwrap() as Date;
    return date.valueOf() === 1;
}

function TestFunctionArgTypeConflictError(): boolean {
    const error = functionArgTypeConflictError.invoke().unwrap() as Error;
    const EXPECTED_STRING = 'foo';

    return error.message === EXPECTED_STRING;
}

function TestFunctionArgTypeConflictMap(): boolean {
    const dataMap = functionArgTypeConflictMap.invoke().unwrap() as Map<string, string>;
    const KEY = 'foo';
    const EXPECTED_VALUE = 'bar';

    return dataMap.get(KEY) === EXPECTED_VALUE;
}

function TestFunctionArgTypeConflictObject(): boolean {
    const obj = functionArgTypeConflictObject.invoke().unwrap() as Object;
    return typeof obj === 'object' && obj instanceof Object;
}

function TestFunctionArgTypeConflictString(): boolean {
    const value = functionArgTypeConflictString.invoke().unwrap() as string;
    const EXPECTED_STRING = 'foo';

    return value === EXPECTED_STRING;
}
