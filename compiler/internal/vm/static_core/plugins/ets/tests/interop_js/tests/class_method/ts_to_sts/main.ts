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

export const tsInt = 1;

export class UserClass {
    value = tsInt;

    get(): number {
        return this.value;
    }

    compare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
}

export function createUserClassFromTs(): UserClass {
    return new UserClass();
}

export const instanceUserClass = new UserClass();

export class ChildClass extends UserClass {
    childMethodCompare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
};

export function createChildClassFromTs(): ChildClass {
    return new ChildClass();
}

export const instanceChildClass = new ChildClass();

interface IForClass {
    value: number;

    get(): number;

    compare(x): boolean
}

export class InterfaceClass implements IForClass {
    value = tsInt;

    get(): number {
        return this.value;
    }

    compare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
}

export function createInterfaceClassFromTs(): InterfaceClass {
    return new InterfaceClass();
}

export const instanceInterfaceClass = new InterfaceClass();

export class StaticClass {
    static value = tsInt;

    static get(): number {
        return this.value;
    }

    static compare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
}

export function createStaticClassFromTs(): StaticClass {
    return StaticClass;
}

export class PrivateClass {
    private value = tsInt;

    private get(): number {
        return this.value;
    }

    private compare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
}

export function createPrivateClassFromTs(): PrivateClass {
    return new PrivateClass();
}

export const instancePrivateClass = new PrivateClass();

export class ProtectedClass {
    protected value = tsInt;

    protected get(): number {
        return this.value;
    }

    protected compare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
}

export function createProtectedClassFromTs(): ProtectedClass {
    return new ProtectedClass();
}

export const instanceProtectedClass = new ProtectedClass();

export class ChildProtectedClass extends ProtectedClass {}

export function createChildProtectedClassFromTs(): ChildProtectedClass {
    return new ChildProtectedClass();
}

export const instanceChildProtectedClass = new ProtectedClass();

abstract class AbstractClass {
    value = tsInt;

    get(): number {
        return this.value;
    }

    compare(x): boolean {
        if (typeof x === 'function' && x === this.get) {
            return true;
        }

        return false;
    }
}

export class ChildAbstractClass extends AbstractClass {};

export function createChildAbstractClassFromTs(): ChildAbstractClass {
    return new ChildAbstractClass();
}

export const instanceChildAbstractClass = new ChildAbstractClass();
