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

let module = ESValue.load('js_code');
let funcReturnTypeNull = module.getProperty('funcReturnTypeNull');
let funcReturnTypeUndefined = module.getProperty('funcReturnTypeUndefined');
let funcArgType = module.getProperty('funcArgType');

function TestFuncCallArg(): boolean {
    if (module.isUndefined() || module.isNull()) {
        return false;
    }
    if (funcReturnTypeNull.isUndefined() || funcReturnTypeNull.isNull()) {
        return false;
    }
    if (funcReturnTypeUndefined.isUndefined() || funcReturnTypeUndefined.isNull()) {
        return false;
    }
    if (funcArgType.isUndefined() || funcArgType.isNull()) {
        return false;
    }

    try {
        const result1 = funcReturnTypeNull.invoke();
        if (!result1.isNull()) {
            return false;
        }

        const result2 = funcReturnTypeUndefined.invoke();
        if (!result2.isUndefined()) {
            return false;
        }

        const result3 = funcArgType.invoke(ESValue.Undefined);
        if (!result3.isUndefined()) {
            return false;
        }

        const result4 = funcArgType.invoke(ESValue.Null);
        if (!result4.isNull()) {
            return false;
        }

        return true;

    } catch (e) {
        return false;
    }
}