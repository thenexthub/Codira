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
let checkIntersectionInterface = module.getProperty('checkIntersectionInterface');
let AgeNameInterfaceClass = module.getProperty('AgeNameInterfaceClass');
let instanceAgeNameInterfaceClass = module.getProperty('instanceAgeNameInterfaceClass');
let ChildAgeNameInterfaceClass = module.getProperty('ChildAgeNameInterfaceClass');
let instanceChildAgeNameInterfaceClass = module.getProperty('instanceChildAgeNameInterfaceClass');

class AgeNameWrong {
    age = tsNumber;
}
function checkArgFuFromSts(): boolean {

    return checkIntersectionInterface.invoke(new AgeNameWrong()).toBoolean() == false;
}

function checkAgeNameClass(): boolean {
    try {
        AgeNameInterfaceClass.instantiate(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkAgeNameClassMethod(): boolean {
    try {
        const AgeName = AgeNameInterfaceClass.instantiate(tsString, tsNumber);
        AgeName.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceAgeNameInterfaceClass(): boolean {
    try {
        instanceAgeNameInterfaceClass.invokeMethod('createUser',tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildAgeNameClass(): boolean {
    try {
        ChildAgeNameInterfaceClass.instantiate(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkChildAgeNameClassMethod(): boolean {
    try {
        const AgeName = ChildAgeNameInterfaceClass.instantiate(tsString, tsNumber);
        AgeName.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceChildAgeNameInterfaceClass(): boolean {
    try {
        instanceChildAgeNameInterfaceClass.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
