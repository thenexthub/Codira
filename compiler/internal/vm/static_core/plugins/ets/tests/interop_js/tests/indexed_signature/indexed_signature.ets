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

let module1 = ESValue.load('utils_js')
let createProxy = module1.getProperty('createProxy');

const stsInt = 2;
const arr: FixedArray<int> = [];

// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkValueDynamicObject(): boolean {
//     const proxy = createProxy(SimpleObject, handler)

//     return proxy['int'] as int == tsInt as int;
// }

// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkAddNewPropertyDynamicObject(): boolean {
//     const proxy = createProxy(SimpleObject, handler)

//     proxy['stsInt'] = stsInt;

//     return proxy['stsInt'] as int == stsInt as int;
// }

// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkAddArrayDynamicObject(): boolean {
//     const proxy = createProxy(SimpleObject, handler)

//     proxy['array'] = arr;

//     return proxy['array'] as Array<int> instanceof Array;
// }

// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkPushToArrayDynamicObject(): boolean {
//     const proxy = createProxy(SimpleObject, handler)

//     proxy['array'] = arr;

//     proxy['array'].push(stsInt);

//     return proxy['array'] as Array<int> instanceof Array && proxy['array'][0] as int == stsInt as int;
// }

// NOTE: (alexanderpolenov) issue(18239) access by number index
// function checkAddToArrayByIndexDynamicObject(): boolean {
//     const proxy = createProxy(SimpleObject, handler)
//     proxy[0] = 1
    
//     return proxy['items'] as Array<int> instanceof Array && proxy['items'][0] as int == stsInt as int;
// }

// NOTE: (alexanderpolenov) issue(18238) access by string index
// function checkCallFunctionDynamicObject(): boolean {
//     const proxy = createProxy(SimpleObject, handler)

//     return proxy['add'] as int == stsInt + stsInt as int;
// }
