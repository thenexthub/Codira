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

let module = ESValue.load('js_code');
let functionReturnArray1 = module.getProperty('functionReturnArray1');
let functionReturnArray2 = module.getProperty('functionReturnArray2');
let functionReturnArray3 = module.getProperty('functionReturnArray3');
let functionReturnString1 = module.getProperty('functionReturnString1');
let functionReturnString2 = module.getProperty('functionReturnString2');
let functionReturnString3 = module.getProperty('functionReturnString3');

let arr: number[] = [3, 4];
let arrString: string[] = ['c', 'd'];

function testFunction(): boolean {
    let ret: boolean = true;

    let ret1 = functionReturnArray1.invoke();
    ret = ret && ret1.getProperty(0).toNumber() === 1 && ret1.getProperty(1).toNumber() === 2;

    let ret2 = functionReturnArray1.invoke(ESValue.wrap(arr));
    ret = ret && ret2.getProperty(0).toNumber() === 3 && ret2.getProperty(1).toNumber() === 4;

    let ret3 = functionReturnArray2.invoke(ESValue.wrap(arr));
    ret = ret && ret3.getProperty(0).toNumber() === 3 && ret3.getProperty(1).toNumber() === 4;

    let ret4 = functionReturnArray3.invoke();
    ret = ret && ret4.getProperty(0).toNumber() === 5 && ret4.getProperty(1).toNumber() === 6;

    let ret5 = functionReturnArray3.invoke(ESValue.wrap(arr));
    ret = ret && ret5.getProperty(0).toNumber() === 3 && ret5.getProperty(1).toNumber() === 4;

    let ret6 = functionReturnString1.invoke();
    ret = ret && ret6.getProperty(0).toString() === 'a' && ret6.getProperty(1).toString() === 'b';

    let ret7 = functionReturnString1.invoke(ESValue.wrap(arrString));
    ret = ret && ret7.getProperty(0).toString() === 'c' && ret7.getProperty(1).toString() === 'd';

    let ret8 = functionReturnString2.invoke(ESValue.wrap(arrString));
    ret = ret && ret8.getProperty(0).toString() === 'c' && ret8.getProperty(1).toString() === 'd';

    let ret9 = functionReturnString3.invoke();
    ret = ret && ret9.getProperty(0).toString() === 'a' && ret9.getProperty(1).toString() === 'b';

    let ret10 = functionReturnString3.invoke(ESValue.wrap(arrString));
    ret = ret && ret10.getProperty(0).toString() === 'c' && ret10.getProperty(1).toString() === 'd';

    return ret;
}
