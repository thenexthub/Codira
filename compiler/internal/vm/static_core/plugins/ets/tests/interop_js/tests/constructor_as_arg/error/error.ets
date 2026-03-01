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
let ParentClass = module.getProperty('ParentClass');
let ChildClass = module.getProperty('ChildClass');
let MethodClass = module.getProperty('MethodClass');
let CreateClassFunction = module.getProperty('CreateClassFunction');

const num = ESValue.wrapNumber(1);
const str = ESValue.wrapString('str');
const foo = global.getProperty('eval').invoke(ESValue.wrapString('() => {}'));
const obj = ESValue.instantiateEmptyObject();
const arr = ESValue.instantiateEmptyArray();

function checkCreateClassFunctionInt(): boolean {
    try {
        CreateClassFunction.invoke(num, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionString(): boolean {
    try {
        CreateClassFunction.invoke(str, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionObj(): boolean {
    try {
        CreateClassFunction.invoke(obj, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionArr(): boolean {
    try {
        CreateClassFunction.invoke(arr, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassFunctionArrow(): boolean {
    try {
        CreateClassFunction.invoke(foo, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionInt(): boolean {
    try {
        CreateClassFunction.invoke(num, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionString(): boolean {
    try {
        CreateClassFunction.invoke(str, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionObj(): boolean {
    try {
        CreateClassFunction.invoke(obj, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionArr(): boolean {
    try {
        CreateClassFunction.invoke(arr, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkCreateClassArrowFunctionArrow(): boolean {
    try {
        CreateClassFunction.invoke(foo, tsInt);
        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassInt(): boolean {
    try {
        ParentClass.instantiate(num);
        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassString(): boolean {
    try {
        ParentClass.instantiate(str);
        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassArray(): boolean {
    try {
        ParentClass.instantiate(arr);
        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassObj(): boolean {
    try {
        ParentClass.instantiate(obj);
        return false;
    } catch (e) {
        return true;
    }
}

function checkParentClassArrow(): boolean {
    try {
        ParentClass.instantiate(foo);
        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassInt(): boolean {
    try {
        ChildClass.instantiate(num);
        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassString(): boolean {
    try {
        ChildClass.instantiate(str);
        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassArray(): boolean {
    try {
        ChildClass.instantiate(arr);
        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassObj(): boolean {
    try {
        ChildClass.instantiate(obj);
        return false;
    } catch (e) {
        return true;
    }
}

function checkChildClassArrow(): boolean {
    try {
        ChildClass.instantiate(foo);
        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassInt(): boolean {
    try {
        const TSClass = MethodClass.instantiate();
        TSClass.invokeMethod('init', num, num);
        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassString(): boolean {
    try {
        const TSClass = MethodClass.instantiate();
        TSClass.invokeMethod('init', str, num);
        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassArr(): boolean {
    try {
        const TSClass = MethodClass.instantiate();
        TSClass.invokeMethod('init', arr, num);
        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassObj(): boolean {
    try {
        const TSClass = MethodClass.instantiate();
        TSClass.invokeMethod('init', obj, num);
        return false;
    } catch (e) {
        return true;
    }
}

function checkMethodClassArrow(): boolean {
    try {
        const TSClass = MethodClass.instantiate();
        TSClass.invokeMethod('init', foo, num);
        return false;
    } catch (e) {
        return true;
    }
}