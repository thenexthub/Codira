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
let ChildAbstractClass = module.getProperty('ChildAbstractClass');
let createChildAbstractClassFromTs = module.getProperty('createChildAbstractClassFromTs');
let instanceChildAbstractClass = module.getProperty('instanceChildAbstractClass');

function checkAbstractClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildAbstractClass.isUndefined() || ChildAbstractClass.isNull()) {
            return false;
        }

        const TSClass = ChildAbstractClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const getMethod = TSClass.getPropertySafe('get');
        if (getMethod.isUndefined()) {
            return false;
        }

        const compareMethod = TSClass.getPropertySafe('compare');
        if (compareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', getMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
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

        if (createChildAbstractClassFromTs.isUndefined() || createChildAbstractClassFromTs.isNull()) {
            return false;
        }

        const TSClass = createChildAbstractClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const getMethod = TSClass.getPropertySafe('get');
        if (getMethod.isUndefined()) {
            return false;
        }

        const compareMethod = TSClass.getPropertySafe('compare');
        if (compareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', getMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
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

        if (instanceChildAbstractClass.isUndefined() || instanceChildAbstractClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildAbstractClass;

        const getMethod = TSClass.getPropertySafe('get');
        if (getMethod.isUndefined()) {
            return false;
        }

        const compareMethod = TSClass.getPropertySafe('compare');
        if (compareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', getMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
        }

        return false;

    } catch (e) {
        return false;
    }
}