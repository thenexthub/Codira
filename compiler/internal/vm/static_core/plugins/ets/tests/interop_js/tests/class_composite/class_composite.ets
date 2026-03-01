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

function Test1(): int {
    let a = ESValue.instantiateEmptyObject();
    a.setProperty('name', 'Edalyn');
    a.setProperty('age', 30);
    a.setProperty('location', 'Boiling Isles');

    let nameValue = a.getProperty('name');
    if (!nameValue.isString() || nameValue.toString() !== 'Edalyn') {
        return 1;
    }

    let ageValue = a.getProperty('age');
    if (!ageValue.isNumber() || ageValue.toNumber() !== 30) {
        return 2;
    }

    let locationValue = a.getProperty('location');
    if (!locationValue.isString() || locationValue.toString() !== 'Boiling Isles') {
        return 3;
    }

    return 0;
}

function Test2(): int {
    let a = ESValue.instantiateEmptyObject();
    a.setProperty('name', 'Luz');
    a.setProperty('age', 13);
    a.setProperty('location', 'Boiling Isles');

    a.setProperty('name', 'king');
    let nameValue = a.getProperty('name');
    if (!nameValue.isString() || nameValue.toString() !== 'king') {
        return 1;
    }

    a.setProperty('age', 5);
    let ageValue = a.getProperty('age');
    if (!ageValue.isNumber() || ageValue.toNumber() !== 5) {
        return 2;
    }

    let locationValue = a.getProperty('location');
    if (!locationValue.isString() || locationValue.toString() !== 'Boiling Isles') {
        return 3;
    }

    return 0;
}