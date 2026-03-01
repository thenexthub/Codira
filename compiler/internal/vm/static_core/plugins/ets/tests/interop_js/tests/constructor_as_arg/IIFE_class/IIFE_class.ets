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
let IIFECreateClassMain = module.getProperty('IIFECreateClassMain');
let MainClass = module.getProperty('MainClass');
let IIFECreateClassAnonymous = module.getProperty('IIFECreateClassAnonymous');
let AnonymousClass = module.getProperty('AnonymousClass');
let IIFECreateClass = module.getProperty('IIFECreateClass');
let IIFEClass = module.getProperty('IIFEClass');
let checkInstance = module.getProperty('checkInstance');

function checkIIFECreateClassMain(): boolean {
    try {
        const TSClass = IIFECreateClassMain;

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

function checkIIFECreateClassAnonymous(): boolean {
    try {
        const TSClass = IIFECreateClassAnonymous;

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

function checkIIFECreateClassIIFE(): boolean {
    try {
        const TSClass = IIFECreateClass;

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
