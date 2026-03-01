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
let MethodClass = module.getProperty('MethodClass');
let ParentClass = module.getProperty('ParentClass');
let ChildClass = module.getProperty('ChildClass');
let AnonymousClass = module.getProperty('AnonymousClass');
let IIFEClass = module.getProperty('IIFEClass');
let checkInstance = module.getProperty('checkInstance');

function checkMethodConstructorMainClass(): boolean {
    try {
        const MClass = MethodClass.instantiate();
        const TSClass = MClass.invokeMethod('init', MainClass, tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(MainClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const value = TSClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkMethodClassParentClass(): boolean {
    try {
        const MClass = MethodClass.instantiate();
        const TSClass = MClass.invokeMethod('init', ParentClass, MainClass);

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

function checkMethodClassChildClass(): boolean {
    try {
        const MClass = MethodClass.instantiate();
        const TSClass = MClass.invokeMethod('init', ChildClass, MainClass);

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

function checkMethodClassAnonymousClass(): boolean {
    try {
        const MClass = MethodClass.instantiate();
        const TSClass = MClass.invokeMethod('init', AnonymousClass, tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(AnonymousClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const value = TSClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkMethodClassIIFEClass(): boolean {
    try {
        const MClass = MethodClass.instantiate();
        const TSClass = MClass.invokeMethod('init', IIFEClass, tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const checkResult = checkInstance.invoke(IIFEClass, TSClass);
        if (!checkResult.isBoolean() || !checkResult.toBoolean()) {
            return false;
        }

        const value = TSClass.getPropertySafe('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}
