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
let instanceAgeNameUnionClass = module.getProperty('instanceAgeNameUnionClass');
let ChildAgeNameUnionClass = module.getProperty('ChildAgeNameUnionClass');
let instanceChildAgeNameUnionClass = module.getProperty('instanceChildAgeNameUnionClass');

class IntersectionWrong {
    age = tsNumber;
}
function checkArgFuFromSts(): boolean {

    return checkUnionType.invoke(new IntersectionWrong()).toBoolean() == false;
}


function checkIntersectionClass(): boolean {
    try {
        AgeNameUnionClass.instantiate(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkIntersectionClassMethod(): boolean {
    try {
        const Intersection = AgeNameUnionClass.instantiate(tsString, tsNumber);
        Intersection.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceIntersectionTypeClass(): boolean {
    try {
        instanceAgeNameUnionClass.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildIntersectionClass(): boolean {
    try {
        ChildAgeNameUnionClass.instantiate(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkChildIntersectionClassMethod(): boolean {
    try {
        const Intersection = ChildAgeNameUnionClass.instantiate(tsString, tsNumber);
        Intersection.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceChildIntersectionTypeClass(): boolean {
    try {
        instanceChildAgeNameUnionClass.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}