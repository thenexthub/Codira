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

class StructCommon {
// TODO: inheriting static invoke with type parameters causes es2panda crash
/*
    static _invoke<T extends StructCommon>(factory: () => T, content: () => void): T {
        console.println("Executing invoke")
        const instance = factory()
        instance.__initializeStruct()
        content()
        return instance
    }
*/
    __initializeStruct(): StructCommon {
        return this
    }
    width(value: string): StructCommon {
        console.println("Setting width to " + value)
        return this
    }
    height(value: string): StructCommon {
        console.println("Setting height to " + value)
        return this
    }
    color(value: string): StructCommon {
        console.println("Setting color to " + value)
        return this
    }
}

class Column extends StructCommon {
    // TODO: this should be taken from the base class
    static $_instantiate(factory: () => Column, content: () => void): Column {
        console.println("Executing invoke")
        const instance = factory()
        instance.__initializeStruct()
        content()
        return instance
    }
    constructor() {
        console.println("Column instantiated")
    }
    override __initializeStruct(): StructCommon {
        console.println("Column initialized")
        return this
    }
}

class Button extends StructCommon {
    // TODO: this should be taken from the base class
    static $_instantiate(factory: () => Button, content: () => void): Button {
        console.println("Executing invoke")
        const instance = factory()
        instance.__initializeStruct()
        content()
        return instance
    }
    constructor() {
        console.println("Button instantiated")
    }
    override __initializeStruct(): StructCommon {
        console.println("Button initialized")
        return this
    }
}
export function main() {
    console.println("Start")

    Column() {
        console.println("Column content")
        // TODO: Calling the Button() { ...
        // causes the compiler to crash
        Button() {
             console.println("Button content")
        }
        .color("#00ff00ff")
    }
    .width('100%')
    .height('100%')
}

