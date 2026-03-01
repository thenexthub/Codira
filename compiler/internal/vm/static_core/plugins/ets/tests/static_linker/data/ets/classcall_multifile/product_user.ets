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

import { User } from './user_dependency';
 
export class Product {
    private name: string;
    private price: number;
    private owner: User;
 
    constructor(name: string, price: number, owner: User) {
        this.name = name;
        this.price = price;
        this.owner = owner;
    }
 
    displayInfo(): void {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log(`Product: ${this.name}, Price: $${this.price}`);
        this.owner.introduce();
    }
}

export class UnusedProductClass {
    public static readonly UNUSED_CONSTANT = 100;

    private unusedMethod(): number {
        return 42;
    }
}