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
let checkLiteralType = module.getProperty('checkLiteralType');
let AgeNameLiteralClass = module.getProperty('AgeNameLiteralClass');
let createAgeNameLiteralClassFromTs = module.getProperty('createAgeNameLiteralClassFromTs');
let callMethodAgeNameLiteralClassFromTs = module.getProperty('callMethodAgeNameLiteralClassFromTs');
let instanceAgeNameLiteralClass = module.getProperty('instanceAgeNameLiteralClass');
let ChildAgeNameLiteralClass = module.getProperty('ChildAgeNameLiteralClass');
let createChildAgeNameLiteralClassFromTs = module.getProperty('createChildAgeNameLiteralClassFromTs');
let callMethodChildAgeNameLiteralClassFromTs = module.getProperty('callMethodChildAgeNameLiteralClassFromTs');
let instanceChildAgeNameLiteralClass = module.getProperty('instanceChildAgeNameLiteralClass');

class Intersection {
    age = tsNumber;
    name = tsString;
}
function checkArgFuFromSts(): boolean {

    return checkLiteralType.invoke(new Intersection()).toBoolean();
}

function checkIntersectionTypeClass(): boolean {
    AgeNameLiteralClass.instantiate(tsString, tsNumber);

    return true;
}

function checkIntersectionClassMethod(): boolean {
    const Intersection = AgeNameLiteralClass.instantiate(tsString, tsNumber);
    Intersection.invokeMethod('createUser', tsString, tsNumber);

    return true;
}

function checkCreateIntersectionClassTypeFromTs(): boolean {
    return createAgeNameLiteralClassFromTs.invoke().toBoolean();
}

function checkCallMethodIntersectionClassTypeFromTs(): boolean {
    return callMethodAgeNameLiteralClassFromTs.invoke().toBoolean();
}

function checkInstanceIntersectionTypeClass(): boolean {
    instanceAgeNameLiteralClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}

function checkChildIntersectionTypeClass(): boolean {
    ChildAgeNameLiteralClass.instantiate(tsString, tsNumber);

    return true;
}

function checkChildIntersectionTypeClassMethod(): boolean {
    const Intersection = ChildAgeNameLiteralClass.instantiate(tsString, tsNumber);
    Intersection.invokeMethod('createUser', tsString, tsNumber);

    return true;
}

function checkCreateChildIntersectionClassTypeFromTs(): boolean {
    return createChildAgeNameLiteralClassFromTs.invoke().toBoolean();
}

function checkCallMethodChildIntersectionClassTypeFromTs(): boolean {
    return callMethodChildAgeNameLiteralClassFromTs.invoke().toBoolean();
}

function checkInstanceChildIntersectionTypeClass(): boolean {
    instanceChildAgeNameLiteralClass.invokeMethod('createUser', tsString, tsNumber);
    return true;
}
