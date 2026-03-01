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

let module = ESValue.load('main_js');
let UserClass = module.getProperty('UserClass');
let createUserClassFromTs = module.getProperty('createUserClassFromTs');
let instanceUserClass = module.getProperty('instanceUserClass');
let ChildClass = module.getProperty('ChildClass');
let createChildClassFromTs = module.getProperty('createChildClassFromTs');
let instanceChildClass = module.getProperty('instanceChildClass');
let InterfaceClass = module.getProperty('InterfaceClass');
let createInterfaceClassFromTs = module.getProperty('createInterfaceClassFromTs');
let instanceInterfaceClass = module.getProperty('instanceInterfaceClass');
let StaticClass = module.getProperty('StaticClass');
let createStaticClassFromTs = module.getProperty('createStaticClassFromTs');
let PrivateClass = module.getProperty('PrivateClass');
let createPrivateClassFromTs = module.getProperty('createPrivateClassFromTs');
let instancePrivateClass = module.getProperty('instancePrivateClass');
let ChildProtectedClass = module.getProperty('ChildProtectedClass');
let createChildProtectedClassFromTs = module.getProperty('createChildProtectedClassFromTs');
let instanceChildProtectedClass = module.getProperty('instanceChildProtectedClass');
let ProtectedClass = module.getProperty('ProtectedClass');
let createProtectedClassFromTs = module.getProperty('createProtectedClassFromTs');
let instanceProtectedClass = module.getProperty('instanceProtectedClass');
let ChildAbstractClass = module.getProperty('ChildAbstractClass');
let createChildAbstractClassFromTs = module.getProperty('createChildAbstractClassFromTs');
let instanceChildAbstractClass = module.getProperty('instanceChildAbstractClass');

function checkChildClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate();
        const AnotherClass = InterfaceClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassFromTs.isUndefined() || createChildClassFromTs.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = createChildClassFromTs.invoke();
        const AnotherClass = InterfaceClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceChildClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceChildClass.isUndefined() || instanceChildClass.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildClass;
        const AnotherClass = InterfaceClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkChildClassMethodOwnMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate();
        const AnotherClass = InterfaceClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('childMethodCompare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassFromTsOwnMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassFromTs.isUndefined() || createChildClassFromTs.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = createChildClassFromTs.invoke();
        const AnotherClass = InterfaceClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('childMethodCompare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceChildClassOwnMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceChildClass.isUndefined() || instanceChildClass.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildClass;
        const AnotherClass = InterfaceClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('childMethodCompare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkUserClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (UserClass.isUndefined() || UserClass.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = UserClass.instantiate();
        const AnotherClass = InterfaceClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateUserClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createUserClassFromTs.isUndefined() || createUserClassFromTs.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = createUserClassFromTs.invoke();
        const AnotherClass = InterfaceClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceUserClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceUserClass.isUndefined() || instanceUserClass.isNull() || InterfaceClass.isUndefined() || InterfaceClass.isNull()) {
            return false;
        }

        const TSClass = instanceUserClass;
        const AnotherClass = InterfaceClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInterfaceClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (InterfaceClass.isUndefined() || InterfaceClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = InterfaceClass.instantiate();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateInterfaceClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createInterfaceClassFromTs.isUndefined() || createInterfaceClassFromTs.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createInterfaceClassFromTs.invoke();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceInterfaceClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceInterfaceClass.isUndefined() || instanceInterfaceClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = instanceInterfaceClass;
        const AnotherClass = UserClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkStaticClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (StaticClass.isUndefined() || StaticClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = StaticClass;
        const AnotherClass = UserClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateStaticClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createStaticClassFromTs.isUndefined() || createStaticClassFromTs.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createStaticClassFromTs.invoke();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkPrivateClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (PrivateClass.isUndefined() || PrivateClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = PrivateClass.instantiate();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreatePrivateClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createPrivateClassFromTs.isUndefined() || createPrivateClassFromTs.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createPrivateClassFromTs.invoke();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstancePrivateClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instancePrivateClass.isUndefined() || instancePrivateClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = instancePrivateClass;
        const AnotherClass = UserClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkChildProtectedClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildProtectedClass.isUndefined() || ChildProtectedClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = ChildProtectedClass.instantiate();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateChildProtectedClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildProtectedClassFromTs.isUndefined() || createChildProtectedClassFromTs.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createChildProtectedClassFromTs.invoke();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceChildProtectedClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceChildProtectedClass.isUndefined() || instanceChildProtectedClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildProtectedClass;
        const AnotherClass = UserClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkProtectedClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ProtectedClass.isUndefined() || ProtectedClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = ProtectedClass.instantiate();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateProtectedClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createProtectedClassFromTs.isUndefined() || createProtectedClassFromTs.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createProtectedClassFromTs.invoke();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceProtectedClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceProtectedClass.isUndefined() || instanceProtectedClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = instanceProtectedClass;
        const AnotherClass = UserClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkAbstractClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildAbstractClass.isUndefined() || ChildAbstractClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = ChildAbstractClass.instantiate();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateAbstractClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildAbstractClassFromTs.isUndefined() || createChildAbstractClassFromTs.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createChildAbstractClassFromTs.invoke();
        const AnotherClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull() || AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkInstanceAbstractClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceChildAbstractClass.isUndefined() || instanceChildAbstractClass.isNull() || UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildAbstractClass;
        const AnotherClass = UserClass.instantiate();

        if (AnotherClass.isUndefined() || AnotherClass.isNull()) {
            return false;
        }

        const anotherGetMethod = AnotherClass.getPropertySafe('get');
        if (anotherGetMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', anotherGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean() == false;
        }

        return false;

    } catch (e) {
        return false;
    }
}