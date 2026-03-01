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
    constructor(manufacturer: int, model: int, ecoFriendly: String, highPerformance: String) {
        this.manufacturer = manufacturer;
        this.__model = model;
        this.__ecoFriendly = ecoFriendly;
        this.highPerformance = highPerformance;
    }

    set model(model: int) {
        this.__model = model;
    }

    get ecoFriendly(): String {
        return this.__ecoFriendly;
    }

    set ecoFriendly(ecoFriendly: String) {
        this.__ecoFriendly = ecoFriendly;
    }

    manufacturer: int;
    private __model: int;
    private __ecoFriendly: String;
    highPerformance: String;
}

function newCarObject() {
    let var1: String = "Porsche"
    let var2: String = "Porsche 911"
    return new Car(0, 0, var1, var2);
}

interface Iface {
    prop: String
}

class C1 implements Iface {
    public prop: String = "Default"
}

class C2 implements Iface {
    public classprop: String = "Default"

    public get prop(): String {
        return this.classprop
    }

    public set prop(p: String) {
        this.classprop = p
    }
}

function newC1() {
    return new C1();
}

function newC2() {
    return new C2();
}
