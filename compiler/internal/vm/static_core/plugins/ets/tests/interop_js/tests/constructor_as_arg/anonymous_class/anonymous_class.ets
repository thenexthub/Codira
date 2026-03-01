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
let AnonymousClassCreateClass = module.getProperty('AnonymousClassCreateClass');
let createAnonymousClassCreateClassWithArgFromTs = module.getProperty('createAnonymousClassCreateClassWithArgFromTs');
let createAnonymousClassCreateClassFromTs = module.getProperty('createAnonymousClassCreateClassFromTs');
let anonymousClassCreateMainInstance = module.getProperty('anonymousClassCreateMainInstance');
let createAnonymousClassCreateIIFEClassFromTs = module.getProperty('createAnonymousClassCreateIIFEClassFromTs');
let anonymousClassCreateIIFEInstance = module.getProperty('anonymousClassCreateIIFEInstance');
let checkInstance = module.getProperty('checkInstance');

function checkAnonymousClassCreateClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (AnonymousClassCreateClass.isUndefined() || AnonymousClassCreateClass.isNull() || 
            MainClass.isUndefined() || MainClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = AnonymousClassCreateClass.instantiate(MainClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(AnonymousClassCreateClass, TSClass);
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

function checkCreateAnonymousClassCreateClassWithArgFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createAnonymousClassCreateClassWithArgFromTs.isUndefined() || createAnonymousClassCreateClassWithArgFromTs.isNull() ||
            MainClass.isUndefined() || MainClass.isNull() ||
            AnonymousClassCreateClass.isUndefined() || AnonymousClassCreateClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createAnonymousClassCreateClassWithArgFromTs.invoke(MainClass);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(AnonymousClassCreateClass, TSClass);
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

function checkCreateAnonymousClassCreateClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createAnonymousClassCreateClassFromTs.isUndefined() || createAnonymousClassCreateClassFromTs.isNull() ||
            AnonymousClassCreateClass.isUndefined() || AnonymousClassCreateClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createAnonymousClassCreateClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(AnonymousClassCreateClass, TSClass);
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

function checkAnonymousClassCreateMainInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (anonymousClassCreateMainInstance.isUndefined() || anonymousClassCreateMainInstance.isNull() ||
            AnonymousClassCreateClass.isUndefined() || AnonymousClassCreateClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = anonymousClassCreateMainInstance;

        const checkResult = checkInstance.invoke(AnonymousClassCreateClass, TSClass);
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

function checkCreateAnonymousClassCreateIIFEClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createAnonymousClassCreateIIFEClassFromTs.isUndefined() || createAnonymousClassCreateIIFEClassFromTs.isNull() ||
            AnonymousClassCreateClass.isUndefined() || AnonymousClassCreateClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createAnonymousClassCreateIIFEClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(AnonymousClassCreateClass, TSClass);
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

function checkAnonymousClassCreateIIFEInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (anonymousClassCreateIIFEInstance.isUndefined() || anonymousClassCreateIIFEInstance.isNull() ||
            AnonymousClassCreateClass.isUndefined() || AnonymousClassCreateClass.isNull() ||
            checkInstance.isUndefined() || checkInstance.isNull() ||
            tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = anonymousClassCreateIIFEInstance;

        const checkResult = checkInstance.invoke(AnonymousClassCreateClass, TSClass);
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