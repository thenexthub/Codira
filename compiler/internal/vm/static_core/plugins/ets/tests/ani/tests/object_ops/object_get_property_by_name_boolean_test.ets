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
    constructor(manufacturer: String, model: String, ecoFriendly: boolean, highPerformance: boolean) {
        this.manufacturer = manufacturer;
        this.__model = model;
        this.__ecoFriendly = ecoFriendly;
        this.highPerformance = highPerformance;
    }
    get model(): String {
        return this.__model;
    }
    get ecoFriendly(): boolean {
        return this.__ecoFriendly;
    }
    manufacturer: String;
    private __model: String;
    private __ecoFriendly: boolean;
    highPerformance: boolean;
}

function newCarObject() {
    return new Car("Porsche", "Porsche 911", false, true);
}
