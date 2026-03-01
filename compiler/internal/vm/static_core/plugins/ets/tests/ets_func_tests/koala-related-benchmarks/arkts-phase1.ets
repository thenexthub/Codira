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

// A mock up
interface CustomComponent {}
class ArkComponent {
    width(value: string): ArkComponent {
        console.log("setting width 100%")
        return this
    }
    height(value: string): ArkComponent {
        console.log("setting width 100%")
        return this
    }
    color(value: string): ArkComponent {
        console.log("setting width 100%")
        return this
    }
}

// A mock up
function remember<Value>(exec: () => Value): Value {
    return exec()
}

// A mock up
export class ArkStructCommon extends ArkComponent implements CustomComponent {
}

export abstract class ArkCommonStruct0<T extends ArkCommonStruct0<T>> extends ArkStructCommon {
    /** @memo */
    static _instantiate<S extends ArkCommonStruct0<S>>(
        builder: ((instance: S) => void) | undefined,
        factory: () => S,
        initializers?: S
    ): S {
        const receiver = remember(() => {
            const instance = factory()
            instance.__initializeStruct(initializers)
            return instance;
        })

        receiver._buildWrapper(builder, initializers)
        return receiver
    }
    protected __initializeStruct(initializers: T | undefined): void {}

    /** @memo */
    protected __updateStruct(initializers: T | undefined): void { }

    /** @memo */
    _buildWrapper(
        builder: ((instance: T) => void) | undefined,
        initializers: T | undefined
    ): void {
            this.__updateStruct(initializers)
            this.build(builder)
    }

    /** @memo */
    build(builder: ((instance: T) => void) | undefined): void {
        throw new Error("Called common build()")
    }
}

// A mock up
class Column extends ArkCommonStruct0<Column> {
    build() {
        console.log("Running Column build()")
    }
}

class ArkMyStructComponent extends ArkCommonStruct0<ArkMyStructComponent> {
    __initializeStruct(initializers: ArkMyStructComponent | undefined): void {
        console.log("initializing MyStruct")
    }
    /** @memo */
    build(builder: ((instance: ArkMyStructComponent) => void) | undefined) {
        console.log("Running MyStruct build")
        Column._instantiate(
            instance => instance
                .width('100%')
                .height('50%'),
            () => new Column(),
            undefined,
        )
    }
}

ArkMyStructComponent._instantiate(
    undefined,
    () => new ArkMyStructComponent(),
    {}
)
