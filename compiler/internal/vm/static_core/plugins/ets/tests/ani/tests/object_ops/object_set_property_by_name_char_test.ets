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
    constructor(manufacturer: String, model: String, ecoFriendly: char, highPerformance: char) {
        this.manufacturer = manufacturer;
        this.__model = model;
        this.__ecoFriendly = ecoFriendly;
        this.highPerformance = highPerformance;
    }

    set model(model: String) {
        this.__model = model;
    }

    get ecoFriendly(): char {
        return this.__ecoFriendly;
    }

    set ecoFriendly(ecoFriendly: char) {
        this.__ecoFriendly = ecoFriendly;
    }

    manufacturer: String;
    private __model: String;
    private __ecoFriendly: char;
    highPerformance: char;
}

function newCarObject() {
    let var1: char = Int.toChar(0)
    let var2: char = Int.toChar(0)
    return new Car("Porsche", "Porsche 911", var1, var2);
}

interface Iface {
    prop: char
}

class C1 implements Iface {
    public prop: char = Int.toChar(0)
}

class C2 implements Iface {
    public classprop: char = Int.toChar(0)

    public get prop(): char {
        return this.classprop
    }

    public set prop(p: char) {
        this.classprop = p
    }
}

function newC1() {
    return new C1();
}

function newC2() {
    return new C2();
}
