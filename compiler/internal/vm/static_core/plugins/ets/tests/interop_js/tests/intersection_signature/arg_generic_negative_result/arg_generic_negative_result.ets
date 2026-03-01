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

let module = ESValue.load('main_js');
let tsNumber = module.getProperty('tsNumber').toNumber();
let tsString = module.getProperty('tsString').toString();
let checkAgeNameGeneric = module.getProperty('checkAgeNameGeneric');
let AgeNameGenericClass = module.getProperty('AgeNameGenericClass');
let instanceAgeNameGenericClass = module.getProperty('instanceAgeNameGenericClass');
let ChildAgeNameGenericClass = module.getProperty('ChildAgeNameGenericClass');
let instanceChildAgeNameGenericClass = module.getProperty('instanceChildAgeNameGenericClass');

class TypeWrong {
    age = tsNumber;
}
function checkArgFuFromSts(): boolean {
    return checkAgeNameGeneric.invoke(new TypeWrong()).toBoolean() == false;
}

function checkTypeClass(): boolean {
    return isThrowing(() => { AgeNameGenericClass.instantiate(tsNumber); });
}

function checkClassMethod(): boolean {
    return isThrowing(() => {
        const Type = AgeNameGenericClass.instantiate(tsString, tsNumber);
        Type.invokeMethod('createUser', tsNumber);
    });
}
function checkInstanceClass(): boolean {
    return isThrowing(() => { instanceAgeNameGenericClass.invokeMethod('createUser', tsNumber); });
}

function checkChildClass(): boolean {
    return isThrowing(() => { ChildAgeNameGenericClass.instantiate(tsNumber); });
}

function checkChildClassMethod(): boolean {
    return isThrowing(() => {
        const Type = ChildAgeNameGenericClass.instantiate(tsString, tsNumber);
        Type.invokeMethod('createUser', tsNumber);
    });
}

function checkInstanceChildClassMethod(): boolean {
    return isThrowing(() => { instanceChildAgeNameGenericClass.invokeMethod('createUser', tsNumber); });
}

function isThrowing(f: () => void): boolean {
    try {
        f();
        return false
    } catch (e) {
        return true
    }
}
