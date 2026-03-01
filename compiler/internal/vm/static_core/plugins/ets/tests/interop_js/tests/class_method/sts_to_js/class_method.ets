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

package class_method.test;

const etsInt: int = 1;

class UserClass {
    value = etsInt;

    get(): int {
        return this.value;
    }

    compare(x: UserClass): boolean {
        if (x instanceof UserClass && x.get() == this.get()) {
            return true;
        }

        return false;
    }
}

function createUserClassFromSts(): UserClass {
    return new UserClass();
}

class ChildClass extends UserClass {
    childMethodCompare(x: ChildClass): boolean {
        if (x instanceof ChildClass && x.get() == this.get()) {
            return true;
        }

        return false;
    }
};

function createChildClassFromSts(): ChildClass {
    return new ChildClass();
}

interface IForClass {
    value: int;

    get(): int;

    compare(x: InterfaceClass): boolean
}

class InterfaceClass implements IForClass {
    value = 1;

    get(): int {
        return this.value;
    }

    compare(x: InterfaceClass): boolean {
        if (x instanceof InterfaceClass && x.get() == this.get()) {
            return true;
        }

        return false;
    }
}

function createInterfaceClassFromSts(): InterfaceClass {
    return new InterfaceClass();
}

class PrivateClass {
    private value = etsInt;

    private get(): int {
        return this.value;
    }

    compare(x: PrivateClass): boolean {
        if (x instanceof PrivateClass && x.get() == this.get()) {
            return true;
        }

        return false;
    }
}

function createPrivateClassFromSts(): PrivateClass {
    return new PrivateClass();
}

class ProtectedClass {
    protected value = etsInt;

    protected get(): int {
        return this.value;
    }

    compare(x: ProtectedClass): boolean {
        if (x instanceof ProtectedClass && x.get() == this.get()) {
            return true;
        }

        return false;
    }
}

function createProtectedClassFromSts(): ProtectedClass {
    return new ProtectedClass();
}

class ChildProtectedClass extends ProtectedClass {}

function createChildProtectedClassFromSts(): ProtectedClass {
    return new ProtectedClass();
}

abstract class AbstractClass {
    value = etsInt;

    get(): number {
        return this.value;
    }

    compare(x: AbstractClass): boolean {
        if (x instanceof AbstractClass && x.get() == this.get()) {
            return true;
        }

        return false;
    }
}

class ChildAbstractClass extends AbstractClass {};

function createChildAbstractClassFromSts(): ChildAbstractClass {
    return new ChildAbstractClass();
}