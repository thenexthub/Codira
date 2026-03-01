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
      Protected member variable - polymorphism.
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

class Fish extends Animal {
    protected waterType: string;
    constructor(name: string, waterType: string) {
        super(name);
        this.waterType = waterType;
    }
    public speak(): string {
        return "Fish speak";
    }

    public getWaterType(): string {
        return this.waterType;
    }
}

function main(): void {
    const animal: Animal = new Fish("Dory", "Saltwater")
    arktest.assertEQ((animal as Fish).getWaterType(), "Saltwater");
}