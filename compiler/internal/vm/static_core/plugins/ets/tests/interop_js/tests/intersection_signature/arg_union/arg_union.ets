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
let checkUnionType = module.getProperty('checkUnionType');
let AgeNameUnionClass = module.getProperty('AgeNameUnionClass');
let createAgeNameUnionClassFromTs = module.getProperty('createAgeNameUnionClassFromTs');
let callMethodAgeNameUnionClassFromTs = module.getProperty('callMethodAgeNameUnionClassFromTs');
let instanceAgeNameUnionClass = module.getProperty('instanceAgeNameUnionClass');
let ChildAgeNameUnionClass = module.getProperty('ChildAgeNameUnionClass');
let createChildAgeNameUnionClassFromTs = module.getProperty('createChildAgeNameUnionClassFromTs');
let callMethodChildAgeNameUnionClassFromTs = module.getProperty('callMethodChildAgeNameUnionClassFromTs');
let instanceChildAgeNameUnionClass = module.getProperty('instanceChildAgeNameUnionClass');

class Intersection {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {

    return checkUnionType.invoke(new Intersection()).toBoolean();
}

function checkIntersectionTypeClass(): boolean {
    AgeNameUnionClass.instantiate(tsString, tsNumber);

    return true;
}

function checkIntersectionClassMethod(): boolean {
    const Intersection = AgeNameUnionClass.instantiate(tsString, tsNumber);
    Intersection.invokeMethod('createUser', tsString, tsNumber);

    return true;
}

function checkCreateIntersectionClassTypeFromTs(): boolean {
    return createAgeNameUnionClassFromTs.invoke().toBoolean();
}

function checkCallMethodIntersectionClassTypeFromTs(): boolean {
    return callMethodAgeNameUnionClassFromTs.invoke().toBoolean();
}

function checkInstanceIntersectionTypeClass(): boolean {
    instanceAgeNameUnionClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}

function checkChildIntersectionTypeClass(): boolean {
    ChildAgeNameUnionClass.instantiate(tsString, tsNumber);

    return true;
}

function checkChildIntersectionTypeClassMethod(): boolean {
    const Intersection = ChildAgeNameUnionClass.instantiate(tsString, tsNumber);
    Intersection.invokeMethod('createUser', tsString, tsNumber);

    return true;
}

function checkCreateChildIntersectionClassTypeFromTs(): boolean {
    return createChildAgeNameUnionClassFromTs.invoke().toBoolean();
}

function checkCallMethodChildIntersectionClassTypeFromTs(): boolean {
    return callMethodChildAgeNameUnionClassFromTs.invoke().toBoolean();
}

function checkInstanceChildIntersectionTypeClass(): boolean {
    instanceChildAgeNameUnionClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}
