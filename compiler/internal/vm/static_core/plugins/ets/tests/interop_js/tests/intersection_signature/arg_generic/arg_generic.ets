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
let createAgeNameClassGenericFromTs = module.getProperty('createAgeNameClassGenericFromTs');
let callMethodAgeNameClassGenericFromTs = module.getProperty('callMethodAgeNameClassGenericFromTs');
let instanceAgeNameGenericClass = module.getProperty('instanceAgeNameGenericClass');
let ChildAgeNameGenericClass = module.getProperty('ChildAgeNameGenericClass');
let createChildAgeNameClassGenericFromTs = module.getProperty('createChildAgeNameClassGenericFromTs');
let callMethodChildAgeNameClassGenericFromTs = module.getProperty('callMethodChildAgeNameClassGenericFromTs');
let instanceChildAgeNameGenericClass = module.getProperty('instanceChildAgeNameGenericClass');

class TypeClass {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {

    return checkAgeNameGeneric.invoke(new TypeClass()).toBoolean();
}

function checkTypeClass(): boolean {
    AgeNameGenericClass.instantiate(tsString, tsNumber);

    return true;
}

function checkClassMethod(): boolean {
    const Type = AgeNameGenericClass.instantiate(tsString, tsNumber);
    Type.invokeMethod('createUser', tsString, tsNumber);

    return !Type.isUndefined();
}

function checkCreateClassFromTs(): boolean {
    return createAgeNameClassGenericFromTs.invoke().toBoolean();
}

function checkCallMethodClassFromTs(): boolean {
    return callMethodAgeNameClassGenericFromTs.invoke().toBoolean();
}

function checkInstanceClass(): boolean {
    instanceAgeNameGenericClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}

function checkChildClass(): boolean {
    const Type = ChildAgeNameGenericClass.instantiate(tsString, tsNumber);

    return !Type.isUndefined();
}

function checkChildClassMethod(): boolean {
    const Type = ChildAgeNameGenericClass.instantiate(tsString, tsNumber);
    Type.invokeMethod('createUser', tsString, tsNumber);

    return !Type.isUndefined();
}

function checkCreateChildClassFromTs(): boolean {
    return createChildAgeNameClassGenericFromTs.invoke().toBoolean();
}

function checkCallMethodChildClassFromTs(): boolean {
    return callMethodChildAgeNameClassGenericFromTs.invoke().toBoolean();
}

function checkInstanceChildClass(): boolean {
    instanceChildAgeNameGenericClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}