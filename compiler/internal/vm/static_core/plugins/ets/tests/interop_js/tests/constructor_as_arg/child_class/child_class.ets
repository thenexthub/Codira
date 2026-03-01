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

let global = ESValue.getGlobal();
let gtestEnv = global.getProperty("gtest_env");
let module = ESValue.load('main_js');

let tsInt = module.getProperty('tsInt');
let MainClass = module.getProperty('MainClass');
let AnonymousClass = module.getProperty('AnonymousClass');
let ChildClass = module.getProperty('ChildClass');
let createChildClassWithArgFromTs = module.getProperty('createChildClassWithArgFromTs');
let createChildClassWithMainFromTs = module.getProperty('createChildClassWithMainFromTs');
let childClassWithMainInstance = module.getProperty('childClassWithMainInstance');
let createChildClassWithAnonymousFromTs = module.getProperty('createChildClassWithAnonymousFromTs');
let childClassWithAnonymousInstance = module.getProperty('childClassWithAnonymousInstance');
let createChildClassWithIIFEFromTs = module.getProperty('createChildClassWithIIFEFromTs');
let childClassWithIIFEInstance = module.getProperty('childClassWithIIFEInstance');
let IIFEClass = module.getProperty('IIFEClass');
let checkInstance = module.getProperty('checkInstance');

function checkInheritorClassMain(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull() || 
            MainClass.isUndefined() || MainClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate(MainClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkInheritorClassAnonymous(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull() || 
            AnonymousClass.isUndefined() || AnonymousClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate(AnonymousClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkInheritorClassIIFE(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull() || 
            IIFEClass.isUndefined() || IIFEClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate(IIFEClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassWithArgMainFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassWithArgFromTs.isUndefined() || createChildClassWithArgFromTs.isNull() ||
            MainClass.isUndefined() || MainClass.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassWithArgFromTs.invoke(MainClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassWithArgAnonymousFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassWithArgFromTs.isUndefined() || createChildClassWithArgFromTs.isNull() ||
            AnonymousClass.isUndefined() || AnonymousClass.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassWithArgFromTs.invoke(AnonymousClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassWithArgIIFEFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassWithArgFromTs.isUndefined() || createChildClassWithArgFromTs.isNull() ||
            IIFEClass.isUndefined() || IIFEClass.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassWithArgFromTs.invoke(IIFEClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassWithMainFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassWithMainFromTs.isUndefined() || createChildClassWithMainFromTs.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassWithMainFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkChildClassWithMainInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (childClassWithMainInstance.isUndefined() || childClassWithMainInstance.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = childClassWithMainInstance;

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassWithAnonymousFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassWithAnonymousFromTs.isUndefined() || createChildClassWithAnonymousFromTs.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassWithAnonymousFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkChildClassWithAnonymousInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (childClassWithAnonymousInstance.isUndefined() || childClassWithAnonymousInstance.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = childClassWithAnonymousInstance;

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassWithIIFEFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassWithIIFEFromTs.isUndefined() || createChildClassWithIIFEFromTs.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassWithIIFEFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkChildClassWithIIFEInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (childClassWithIIFEInstance.isUndefined() || childClassWithIIFEInstance.isNull() ||
            ChildClass.isUndefined() || ChildClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = childClassWithIIFEInstance;

        const checkResult = checkInstance.invoke(ChildClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const otherClass = TSClass.getPropertySafe('_otherClass');
        if (otherClass.isUndefined()) {
            return false;
        }

        const value = otherClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}
