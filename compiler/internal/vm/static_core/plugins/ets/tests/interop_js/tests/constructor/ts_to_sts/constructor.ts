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

export const tsInt: number = 1;

export class NamedClass {
    _value: number;

    constructor(value: number) {
        this._value = value;
    }
}

export function createNamedClassFromTs(): NamedClass {
    return new NamedClass(tsInt);
}

export const namedClassInstance = new NamedClass(tsInt);

export const AnonymousClass = class {
    _value: number;

    constructor(public value: number) {
        this._value = value;
    }
};

export function createAnonymousClassFromTs(): InstanceType<typeof AnonymousClass> {
    return new AnonymousClass(tsInt);
}

export const anonymousClassInstance = new AnonymousClass(tsInt);

export function FunctionConstructor(value: number): void {
    this._value = value;
}

export function createFunctionConstructorFromTs(): { _value: number } {
    return new FunctionConstructor(tsInt);
}

export const functionConstructorInstance = new FunctionConstructor(tsInt);

export const IIFEClass: { new(_value: number): { _value: number } } = (function (): { new (_value: number): { _value: number } } {
    return class {
        _value: number;

        constructor(value: number) {
            this._value = value;
        }
    };
})();

export function createIIFEClassFromTs(): { _value: number } {
    return new IIFEClass(tsInt);
}

export const IIFEClassInstance = new IIFEClass(tsInt);

export const IIFEConstructor: (value: number) => void = (function (): (value: number) => void {
    function constructorFoo(this: { _value: number }, value: number): void { 
        this._value = value;
    }

    return constructorFoo;
})();

export function createIIFEConstructorFromTs(): { _value: number } {
    return new IIFEConstructor(tsInt);
}

export const IIFEConstructorInstance = new IIFEConstructor(tsInt);

export class MethodCreateConstructor {
    Constructor(): { new(_value: number) } {
        return class {
            _value: number;

            constructor(value: number) {
                this._value = value;
            }
        };
    }
}

export function createMethodConstructorClass(): MethodCreateConstructor {
    return new MethodCreateConstructor();
}

export const methodConstructorInstance = new MethodCreateConstructor();
export const methodCreateAnonymousClass = methodConstructorInstance.Constructor();
export const methodCreateClassInstance = new methodCreateAnonymousClass(tsInt);

export const SimpleObject = {
    _value: tsInt,
};

export const simpleArrowFunction = (int): { int: number } => {
    return {
        int,
    };
};

abstract class Abstract {
    _value: number;
    constructor(value: number) {
        this._value = value;
    }
}

export class AbstractClass extends Abstract { }

export function createAbstractClassFromTs(): AbstractClass {
    return new AbstractClass(tsInt);
}

export const abstractClassInstance = new AbstractClass(tsInt);

export class ChildClass extends NamedClass {
    constructor(value: number) {
        super(value);
    }
}

export function createChildClassFromTs(): ChildClass {
    return new ChildClass(tsInt);
}

export const childClassInstance = new ChildClass(tsInt);

