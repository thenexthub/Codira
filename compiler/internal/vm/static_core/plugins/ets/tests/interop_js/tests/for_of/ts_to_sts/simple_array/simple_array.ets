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

let module = ESValue.load('simple_array_js')
let emptyArray = module.getProperty('emptyArray');
let numberArray = module.getProperty('numberArray');
let stringArray = module.getProperty('stringArray');

function checkEmptyArray()
{
    let result = true;

    let callback = (arrElem: Any) => {
        result = false;
        console.log('There should be no iterations for an empty array.')
    };

    JSRuntime.iterate(emptyArray, callback);

    return result;
}

function checkNumberArray()
{
    let result = false;
    let idx = 0;
    let expectedArr: number[] = [1, 2, 3, 4]

    let callback = (arrElem: Any) => {
        if (arrElem as number !== expectedArr[idx]) {
            console.log(`Comparison failed:`);
            console.log(`arrElem = ${arrElem}, expectedArr[${idx}] = ${expectedArr[idx]}`);
            result = false;
            return;
        }
        result = true;
        idx++;
    };

    JSRuntime.iterate(numberArray, callback);

    return result;
}

function checkStringArray()
{
    let result = false;
    let idx = 0;
    let expectedArr: string[] = ['Hello', 'World', '!']

    let callback = (arrElem: Any) => {
        if (arrElem as string !== expectedArr[idx]) {
            console.log(`Comparison failed:`);
            console.log(`arrElem = ${arrElem}, expectedArr[${idx}] = ${expectedArr[idx]}`);
            result = false;
            return;
        }
        result = true;
        idx++;
    };

    JSRuntime.iterate(stringArray, callback);

    return result;
}
