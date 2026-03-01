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

class Car {
    constructor(manufacturer: String, model: String, year: short, length: short) {
        this.manufacturer = manufacturer;
        this.__model = model;
        this.length = length;
        this.__year = year;
    }
    get model(): String {
        return this.__model;
    }
    get year(): short {
        return this.__year;
    }
    manufacturer: String;
    private __model: String;
    length: short;
    private __year: short;
}

function newCarObject() {
    const year_: short = 1989;
    const length_: short = 4275;
    return new Car("Porsche", "Porsche 911", year_, length_);
}
