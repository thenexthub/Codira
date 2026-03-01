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
let AgeNameInterfaceClass = module.getProperty('AgeNameInterfaceClass');
let checkIntersectionInterface = module.getProperty('checkIntersectionInterface');
let createAgeNameClassInterfaceFromTs = module.getProperty('createAgeNameClassInterfaceFromTs');
let callMethodAgeNameClassInterfaceFromTs = module.getProperty('callMethodAgeNameClassInterfaceFromTs');
let instanceAgeNameInterfaceClass = module.getProperty('instanceAgeNameInterfaceClass');
let ChildAgeNameInterfaceClass = module.getProperty('ChildAgeNameInterfaceClass');
let createChildAgeNameClassInterfaceFromTs = module.getProperty('createChildAgeNameClassInterfaceFromTs');
let callMethodChildAgeNameClassInterfaceFromTs = module.getProperty('callMethodChildAgeNameClassInterfaceFromTs');
let instanceChildAgeNameInterfaceClass = module.getProperty('instanceChildAgeNameInterfaceClass');

class AgeNameClass {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {
    return checkIntersectionInterface.invoke(new AgeNameClass()).toBoolean();
}

function checkAgeNameClass(): boolean {
    AgeNameInterfaceClass.instantiate(tsString, tsNumber);

    return true;
}

function checkAgeNameClassMethod(): boolean {
    const AgeName = AgeNameInterfaceClass.instantiate(tsString, tsNumber);
    AgeName.invokeMethod('createUser', tsString, tsNumber);

    return true;
}

function checkCreateAgeNameClassInterfaceFromTs(): boolean {
    return createAgeNameClassInterfaceFromTs.invoke().toBoolean();
}

function checkCallMethodAgeNameClassInterfaceFromTs(): boolean {
    return callMethodAgeNameClassInterfaceFromTs.invoke().toBoolean();
}

function checkInstanceAgeNameInterfaceClass(): boolean {
    instanceAgeNameInterfaceClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}

function checkChildAgeNameInterfaceClass(): boolean {
    const AgeName = ChildAgeNameInterfaceClass.instantiate(tsString, tsNumber);

    return true;
}

function checkChildAgeNameInterfaceClassMethod(): boolean {
    const AgeName = ChildAgeNameInterfaceClass.instantiate(tsString, tsNumber);
    AgeName.invokeMethod('createUser', tsString, tsNumber);

    return true;
}

function checkCreateChildAgeNameClassInterfaceFromTs(): boolean {
    return createChildAgeNameClassInterfaceFromTs.invoke().toBoolean();
}

function checkCallMethodChildAgeNameClassInterfaceFromTs(): boolean {
    return callMethodChildAgeNameClassInterfaceFromTs.invoke().toBoolean();
}

function checkInstanceChildAgeNameInterfaceClass(): boolean {
    instanceChildAgeNameInterfaceClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}