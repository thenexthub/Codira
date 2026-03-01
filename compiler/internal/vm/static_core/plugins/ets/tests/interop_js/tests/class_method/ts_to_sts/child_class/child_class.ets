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
let ChildClass = module.getProperty('ChildClass');
let UserClass = module.getProperty('UserClass');
let createChildClassFromTs = module.getProperty('createChildClassFromTs');
let instanceChildClass = module.getProperty('instanceChildClass');

function checkChildClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate();

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

function checkCreateChildClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassFromTs.isUndefined() || createChildClassFromTs.isNull()) {
            return false;
        }

        const TSClass = createChildClassFromTs.invoke();

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

function checkInstanceChildClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceChildClass.isUndefined() || instanceChildClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildClass;

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

function checkChildClassMethodOwnMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull()) {
            return false;
        }

        if (UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate();
        const ParentClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        if (ParentClass.isUndefined() || ParentClass.isNull()) {
            return false;
        }

        const parentGetMethod = ParentClass.getPropertySafe('get');
        if (parentGetMethod.isUndefined()) {
            return false;
        }

        const childMethodCompareMethod = TSClass.getPropertySafe('childMethodCompare');
        if (childMethodCompareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('childMethodCompare', parentGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
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

        if (createChildClassFromTs.isUndefined() || createChildClassFromTs.isNull()) {
            return false;
        }

        if (UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = createChildClassFromTs.invoke();
        const ParentClass = UserClass.instantiate();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        if (ParentClass.isUndefined() || ParentClass.isNull()) {
            return false;
        }

        const parentGetMethod = ParentClass.getPropertySafe('get');
        if (parentGetMethod.isUndefined()) {
            return false;
        }

        const childMethodCompareMethod = TSClass.getPropertySafe('childMethodCompare');
        if (childMethodCompareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('childMethodCompare', parentGetMethod);
        
        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
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

        if (instanceChildClass.isUndefined() || instanceChildClass.isNull()) {
            return false;
        }

        if (UserClass.isUndefined() || UserClass.isNull()) {
            return false;
        }

        const TSClass = instanceChildClass;
        const ParentClass = UserClass.instantiate();

        if (ParentClass.isUndefined() || ParentClass.isNull()) {
            return false;
        }

        const parentGetMethod = ParentClass.getPropertySafe('get');
        if (parentGetMethod.isUndefined()) {
            return false;
        }

        const childMethodCompareMethod = TSClass.getPropertySafe('childMethodCompare');
        if (childMethodCompareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('childMethodCompare', parentGetMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
        }

        return false;

    } catch (e) {
        return false;
    }
}