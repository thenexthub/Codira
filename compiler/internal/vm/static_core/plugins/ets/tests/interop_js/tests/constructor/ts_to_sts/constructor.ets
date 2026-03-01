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
let tsInt = module.getProperty('tsInt');
let NamedClass = module.getProperty('NamedClass');
let createNamedClassFromTs = module.getProperty('createNamedClassFromTs');
let namedClassInstance = module.getProperty('namedClassInstance');
let AnonymousClass = module.getProperty('AnonymousClass');
let createAnonymousClassFromTs = module.getProperty('createAnonymousClassFromTs');
let anonymousClassInstance = module.getProperty('anonymousClassInstance');
let FunctionConstructor = module.getProperty('FunctionConstructor');
let createFunctionConstructorFromTs = module.getProperty('createFunctionConstructorFromTs');
let functionConstructorInstance = module.getProperty('functionConstructorInstance');
let IIFEClass = module.getProperty('IIFEClass');
let createIIFEClassFromTs = module.getProperty('createIIFEClassFromTs');
let IIFEClassInstance = module.getProperty('IIFEClassInstance');
let IIFEConstructor = module.getProperty('IIFEConstructor');
let createIIFEConstructorFromTs = module.getProperty('createIIFEConstructorFromTs');
let IIFEConstructorInstance = module.getProperty('IIFEConstructorInstance');
let MethodCreateConstructor = module.getProperty('MethodCreateConstructor');
let createMethodConstructorClass = module.getProperty('createMethodConstructorClass');
let methodConstructorInstance = module.getProperty('methodConstructorInstance');
let methodCreateAnonymousClass = module.getProperty('methodCreateAnonymousClass');
let methodCreateClassInstance = module.getProperty('methodCreateClassInstance');
let SimpleObject = module.getProperty('SimpleObject');
let simpleArrowFunction = module.getProperty('simpleArrowFunction');
let AbstractClass = module.getProperty('AbstractClass');
let createAbstractClassFromTs = module.getProperty('createAbstractClassFromTs');
let abstractClassInstance = module.getProperty('abstractClassInstance');
let ChildClass = module.getProperty('ChildClass');
let createChildClassFromTs = module.getProperty('createChildClassFromTs');
let childClassInstance = module.getProperty('childClassInstance');

function checkNamedClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (NamedClass.isUndefined() || NamedClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = NamedClass.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateNamedClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createNamedClassFromTs.isUndefined() || createNamedClassFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createNamedClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkNamedClassInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (namedClassInstance.isUndefined() || namedClassInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = namedClassInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkAnonymousClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (AnonymousClass.isUndefined() || AnonymousClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = AnonymousClass.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
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
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createAnonymousClassFromTs.isUndefined() || createAnonymousClassFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createAnonymousClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
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
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (anonymousClassInstance.isUndefined() || anonymousClassInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = anonymousClassInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkFunctionConstructor(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (FunctionConstructor.isUndefined() || FunctionConstructor.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = FunctionConstructor.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateFunctionConstructorFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createFunctionConstructorFromTs.isUndefined() || createFunctionConstructorFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createFunctionConstructorFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkFunctionConstructorInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (functionConstructorInstance.isUndefined() || functionConstructorInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = functionConstructorInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkIIFEClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (IIFEClass.isUndefined() || IIFEClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = IIFEClass.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkIIFEClassError(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (IIFEClass.isUndefined() || IIFEClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = IIFEClass.invoke(tsInt);
        return false;

    } catch (e) {
        console.log(e);
        return true;
    }
}

function checkCreateIIFEClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createIIFEClassFromTs.isUndefined() || createIIFEClassFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createIIFEClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkIIFEClassInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (IIFEClassInstance.isUndefined() || IIFEClassInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = IIFEClassInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkIIFEConstructor(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (IIFEConstructor.isUndefined() || IIFEConstructor.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = IIFEConstructor.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkIIFEConstructorUndefined(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (IIFEConstructor.isUndefined() || IIFEConstructor.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = IIFEConstructor.invoke(tsInt);
        return TSClass.isUndefined();

    } catch (e) {
        return false;
    }
}

function checkCreateIIFEConstructorFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createIIFEConstructorFromTs.isUndefined() || createIIFEConstructorFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createIIFEConstructorFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkIIFEConstructorInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (IIFEConstructorInstance.isUndefined() || IIFEConstructorInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = IIFEConstructorInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}
//NOTE issue(18077) enable after fix 'SyntaxError: Cannot find type' when  try to call method with constructor.
// function checkMethodCreateConstructor(): boolean {
//     const ParentClass = new MethodCreateConstructor();
//     const anonymousClass = ParentClass.Constructor();
//     const TSClass = new anonymousClass(tsInt)

//     return TSClass._value as int == tsInt as int;
// }

//NOTE issue(18077) enable after fix 'SyntaxError: Cannot find type' when  try to call method with constructor.
// function checkCreateMethodConstructorClass(): boolean {
//     const ParentClass = createMethodConstructorClass();
//     const anonymousClass = ParentClass.Constructor();
//     const TSClass = new anonymousClass(tsInt)

//     return TSClass._value as int == tsInt as int;
// }

//NOTE issue(18077) enable after fix 'SyntaxError: Cannot find type' when  try to call method with constructor.
// function checkMethodConstructorInstance(): boolean {
//     const anonymousClass = methodConstructorInstance.Constructor();
//     const TSClass = new anonymousClass(tsInt)

//     return TSClass._value as int == tsInt as int;
// }

function checkMethodCreateAnonymousClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (methodCreateAnonymousClass.isUndefined() || methodCreateAnonymousClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = methodCreateAnonymousClass.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkMethodCreateClassInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (methodCreateClassInstance.isUndefined() || methodCreateClassInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = methodCreateClassInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkSimpleObject(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (SimpleObject.isUndefined() || SimpleObject.isNull()) {
            return false;
        }

        SimpleObject.instantiate();
        return false;

    } catch (e) {
        return true;
    }
}

function checkSimpleArrowFunction(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (simpleArrowFunction.isUndefined() || simpleArrowFunction.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const test = simpleArrowFunction.instantiate(tsInt);
        return false;

    } catch (e) {
        return true;
    }
}

function checkAbstractClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (AbstractClass.isUndefined() || AbstractClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = AbstractClass.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateAbstractClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createAbstractClassFromTs.isUndefined() || createAbstractClassFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createAbstractClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkAbstractInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (abstractClassInstance.isUndefined() || abstractClassInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = abstractClassInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkChildClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ChildClass.isUndefined() || ChildClass.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = ChildClass.instantiate(tsInt);

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkCreateChildClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createChildClassFromTs.isUndefined() || createChildClassFromTs.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const TSClass = createChildClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const value = TSClass.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}

function checkChildInstance(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (childClassInstance.isUndefined() || childClassInstance.isNull() || tsInt.isUndefined() || tsInt.isNull()) {
            return false;
        }

        const value = childClassInstance.getProperty('_value');
        if (value.isUndefined()) {
            return false;
        }

        return value.toNumber() == tsInt.toNumber();

    } catch (e) {
        return false;
    }
}