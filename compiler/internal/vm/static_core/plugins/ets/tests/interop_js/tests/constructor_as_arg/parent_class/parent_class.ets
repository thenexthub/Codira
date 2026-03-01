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
let ParentClass = module.getProperty('ParentClass');
let createClassWithArgFromTs = module.getProperty('createClassWithArgFromTs');
let createMainClassFromTs = module.getProperty('createMainClassFromTs');
let createAnonymousClassFromTs = module.getProperty('createAnonymousClassFromTs');
let anonymousClassInstance = module.getProperty('anonymousClassInstance');
let createIIFEClassFromTs = module.getProperty('createIIFEClassFromTs');
let iifeClassInstance = module.getProperty('iifeClassInstance');
let mainClassInstance = module.getProperty('mainClassInstance');
let IIFEClass = module.getProperty('IIFEClass');
let checkInstance = module.getProperty('checkInstance');

function checkMainClassAsArg(): boolean {
    try {
        const TSClass = ParentClass.instantiate(MainClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkAnonymousClassAsArg(): boolean {
    try {
        const TSClass = ParentClass.instantiate(AnonymousClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkIIFEClassAsArg(): boolean {
    try {
        const TSClass = ParentClass.instantiate(IIFEClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkCreateMainClassWithArgFromTs(): boolean {
    try {
        const TSClass = createClassWithArgFromTs.invoke(MainClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkCreateMainClassFromTs(): boolean {
    try {
        const TSClass = createMainClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkMainClassInstance(): boolean {
    try {
        const TSClass = mainClassInstance;

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkCreateAnonymousClassWithArgFromTs(): boolean {
    try {
        const TSClass = createClassWithArgFromTs.invoke(AnonymousClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkCreateAnonymousClassFromTs(): boolean {
    try {
        const TSClass = createAnonymousClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkAnonymousClassInstance(): boolean {
    try {
        const TSClass = anonymousClassInstance;

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkCreateIifeClassFromTs(): boolean {
    try {
        const TSClass = createIIFEClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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

function checkIifeClassInstance(): boolean {
    try {
        const TSClass = iifeClassInstance;

        const checkResult = checkInstance.invoke(ParentClass, TSClass);
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
