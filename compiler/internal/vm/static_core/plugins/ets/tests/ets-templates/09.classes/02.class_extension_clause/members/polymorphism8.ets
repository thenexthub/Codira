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

/*---
desc: Testing access to member variables in the case of polymorphism.
      Member variable access when testing abstract classes combined with polymorphism.
---*/

class Animal {
    name: string = '';
    constructor(name: string) {
        this.name = name;
    }
    public speak(): string {
        return "Animal speak";
    }
}

abstract class Bird2 extends Animal {
    public abstract fly(): string;
    constructor(name: string) {
        super(name);
    }
}

class Eagle extends Bird2 {
    public fly(): string {
        return "Eagle is flying";
    }
    public speak(): string {
        return "Screech!";
    }
    constructor(name: string) {
        super(name);
    }
}

function main(): void {
    const animal: Animal = new Eagle("Sky");
    arktest.assertEQ("Eagle is flying", (animal as Eagle).fly());
}