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
let instanceAgeNameLiteralClass = module.getProperty('instanceAgeNameLiteralClass');
let ChildAgeNameLiteralClass = module.getProperty('ChildAgeNameLiteralClass');
let instanceChildAgeNameLiteralClass = module.getProperty('instanceChildAgeNameLiteralClass');


class IntersectionWrong {
    age = tsNumber;
}
function checkArgFuFromSts(): boolean {

    return checkLiteralType.invoke(new IntersectionWrong()).toBoolean() == false;
}


function checkIntersectionClass(): boolean {
    try {
        AgeNameLiteralClass.instantiate(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkIntersectionClassMethod(): boolean {
    try {
        const Intersection = AgeNameLiteralClass.instantiate(tsString, tsNumber);
        Intersection.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceIntersectionTypeClass(): boolean {
    try {
        instanceAgeNameLiteralClass.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}

function checkChildIntersectionClass(): boolean {
    try {
        ChildAgeNameLiteralClass.instantiate(tsNumber);

        return false;
    } catch (e) {
        return true;

    }
}

function checkChildIntersectionClassMethod(): boolean {
    try {
        const Intersection = ChildAgeNameLiteralClass.instantiate(tsString, tsNumber);
        Intersection.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}
function checkInstanceChildIntersectionTypeClass(): boolean {
    try {
        instanceChildAgeNameLiteralClass.invokeMethod('createUser', tsNumber);

        return false;
    } catch (e) {
        return true;
    }
}