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
let module = gtestEnv.getProperty("module");

let evaluateNumber = module.getProperty('evaluateNumber');
let ExampleClass = module.getProperty('ExampleClass');
let ClassWithEmptyConstructor = module.getProperty('ClassWithEmptyConstructor');
let evaluateObject = module.getProperty('evaluateObject');
let MyNamespace = module.getProperty('MyNamespace');
let evaluateArray = module.getProperty('evaluateArray');

//! CHECKER      jscallEmpty JIT
//! RUN          force_jit: true, entry: "jscallEmpty", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallEmpty"
//! METHOD       "jscallEmpty"
//! PASS_AFTER   "IrBuilder"

function jscallEmpty(): int {
    try {
        evaluateNumber.invoke()
        return 0
    } catch (e) {
        return 1
    }
}

//! CHECKER      testJSONStringifyESObjectInstance JIT
//! RUN          force_jit: true, entry: "testJSONStringifyESObjectInstance", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::testJSONStringifyESObjectInstance"
//! METHOD       "testJSONStringifyESObjectInstance"
//! PASS_AFTER   "IrBuilder"

function testJSONStringifyESObjectInstance(): boolean {
    return true
}

//! CHECKER      testJSONStringifyESObjectArray JIT
//! RUN          force_jit: true, entry: "testJSONStringifyESObjectArray", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::testJSONStringifyESObjectArray"
//! METHOD       "testJSONStringifyESObjectArray"
//! PASS_AFTER   "IrBuilder"

function testJSONStringifyESObjectArray(): boolean {
    return true
}

//! CHECKER      jsnewEmpty JIT
//! RUN          force_jit: true, entry: "jsnewEmpty", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewEmpty"
//! METHOD       "jsnewEmpty"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyInt"
//! INST_NOT     /JSRuntime.*Value/ # no casts from JSValue to primitive
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 2
//! INST_COUNT   /i32.*Cast f64/, 2

function jsnewEmpty(): int {
    try {
        const resObj = ClassWithEmptyConstructor.instantiate()
        const resGetV0 = resObj.getProperty('v0').toNumber()
        const resGetV1 = resObj.getProperty('v1').toNumber()
        if (resGetV0 == 42 && resGetV1 == 42) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jscallStaticMethodEmpty JIT
//! RUN          force_jit: true, entry: "jscallStaticMethodEmpty", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallStaticMethodEmpty"
//! METHOD       "jscallStaticMethodEmpty"
//! PASS_AFTER   "IrBuilder"

function jscallStaticMethodEmpty(): int {
    try {
        ExampleClass.invokeMethod('emptyMethod')
        return 0
    } catch (e) {
        return 1
    }
}

//! CHECKER      jscallObject JIT
//! RUN          force_jit: true, entry: "jscallObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallObject"
//! METHOD       "jscallObject"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetPropertyDouble"
//! INST_NEXT    /i32.*Cast f64/
//! INST_NEXT    "Intrinsic.JSRuntimeGetPropertyDouble"
//! INST_NEXT    /i32.*Cast f64/

function jscallObject(): int {
    try {
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))
        const res = evaluateObject.invoke(resObj)
        const resGetV0 = res.getProperty('v0').toNumber()
        const resGetV1 = res.getProperty('v1').toNumber()
        if (resGetV0 == 84 && resGetV1 == 84) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jsnewObject JIT
//! RUN          force_jit: true, entry: "jsnewObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewObject"
//! METHOD       "jsnewObject"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4

function jsnewObject(): int {
    try {
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))
        const res = ExampleClass.instantiate(resObj, resObj)
        const resGetV0 = res.getProperty('v0')
        const resGetV1 = res.getProperty('v1')
        if (resGetV0.getProperty('v0').toNumber() == 42 && resGetV0.getProperty('v1').toNumber() == 42 && 
            resGetV1.getProperty('v0').toNumber() == 42 && resGetV1.getProperty('v1').toNumber() == 42) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jsnewSetPropertyObject JIT
//! RUN          force_jit: true, entry: "jsnewSetPropertyObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewSetPropertyObject"
//! METHOD       "jsnewSetPropertyObject"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeSetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyJSValue", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 4

function jsnewSetPropertyObject(): int {
    try {
        const resObj = ClassWithEmptyConstructor.instantiate()
        const res = ClassWithEmptyConstructor.instantiate()
        resObj.setProperty('v0', res)
        resObj.setProperty('v1', res)
        const resGetV0 = resObj.getProperty('v0')
        const resGetV1 = resObj.getProperty('v1')
        if (resGetV0.getProperty('v0').toNumber() == 42 && resGetV0.getProperty('v1').toNumber() == 42 && 
            resGetV1.getProperty('v0').toNumber() == 42 && resGetV1.getProperty('v1').toNumber() == 42) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jscallMethodObject JIT
//! RUN          force_jit: true, entry: "jscallMethodObject", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallMethodObject"
//! METHOD       "jscallMethodObject"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyDouble", 2

function jscallMethodObject(): int {
    try {
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))
        const res = evaluateObject.invoke(resObj)
        const resGetV0 = res.getProperty('v0').toNumber()
        const resGetV1 = res.getProperty('v1').toNumber()
        if (resGetV0 == 84 && resGetV1 == 84) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jscallMethodSimple JIT
//! RUN          force_jit: true, entry: "jscallMethodSimple", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallMethodSimple"
//! METHOD       "jscallMethodSimple"
//! PASS_AFTER   "IrBuilder"

function jscallMethodSimple(): int {
    try {
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))
        const res = resObj.invokeMethod('instanceEvaluateNumber').toNumber()
        if (res == 84) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jscallString JIT
//! RUN          force_jit: true, entry: "jscallString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallString"
//! METHOD       "jscallString"
//! PASS_AFTER   "IrBuilder"
//! INST         "Intrinsic.JSRuntimeGetValueString"
//! INST_NEXT    "Intrinsic.StdCoreStringEquals"

function jscallString(): int {
    try {
        const v0: String = 'hello'
        const v1: String = 'world'
        const res = evaluateNumber.invoke(ESValue.wrapString(v0), ESValue.wrapString(v1)).toString()
        if (res == 'helloworld') {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jsnewString JIT
//! RUN          force_jit: true, entry: "jsnewString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewString"
//! METHOD       "jsnewString"
//! PASS_AFTER   "IrBuilder"
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyString", 2
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetValueString"

function jsnewString(): int {
    try {
        const v0: String = 'hello'
        const v1: String = 'world'
        const resObj = ExampleClass.instantiate(ESValue.wrapString(v0), ESValue.wrapString(v1))
        const resGetV0 = resObj.getProperty('v0').toString()
        const resGetV1 = resObj.getProperty('v1').toString()
        if (resGetV0 == v0 && resGetV1 == v1) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! DISABLED_CHECKER      jsnewSetPropertyString JIT
//! RUN          force_jit: true, entry: "jsnewSetPropertyString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jsnewSetPropertyString"
//! METHOD       "jsnewSetPropertyString"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.JSRuntimeSetPropertyJSValue"
//! INST_COUNT   "Intrinsic.JSRuntimeNewJSValueString", 2
//! INST_COUNT   "Intrinsic.JSRuntimeSetPropertyString", 2
//! INST_COUNT   "Intrinsic.JSRuntimeGetPropertyString", 2
//! INST_NOT     "Intrinsic.JSRuntimeGetPropertyJSValue"
//! INST_NOT     "Intrinsic.JSRuntimeGetValueString"
//! PASS_AFTER   "Cleanup"
//! INST_NOT     "Intrinsic.JSRuntimeNewJSValueString"

function jsnewSetPropertyString(): int {
    try {
        const v0: String = 'hello'
        const v1: String = 'world'
        const resObj = ClassWithEmptyConstructor.instantiate()
        resObj.setProperty('v0', ESValue.wrapString(v0))
        resObj.setProperty('v1', ESValue.wrapString(v1))
        const resGetV0 = resObj.getProperty('v0').toString()
        const resGetV1 = resObj.getProperty('v1').toString()
        if (resGetV0 == v0 && resGetV1 == v1) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

//! CHECKER      jscallStaticMethodString JIT
//! RUN          force_jit: true, entry: "jscallStaticMethodString", options: "--compiler-regex=class_operations_frontend.ETSGLOBAL::jscallStaticMethodString"
//! METHOD       "jscallStaticMethodString"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "Intrinsic.JSRuntimeNewJSValueString"
//! INST    "Intrinsic.JSRuntimeGetValueString"

function jscallStaticMethodString(): int {
    try {
        const v0: String = 'hello'
        const v1: String = 'world'
        const res = ExampleClass.invokeMethod('evaluateNumber', ESValue.wrapString(v0), ESValue.wrapString(v1)).toString()
        if (res == 'helloworld') {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jscallArray(): int {
    try {
        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const res = evaluateArray.invoke(ESValue.wrap(v0), ESValue.wrapNumber(10))
        for (let i = 0; i < 10; ++i) {
            if (res.getProperty(i).toNumber() != 2 * i * i) {
                return 1
            }
        }

        return 0
    } catch (e) {
        return 1
    }
}

function jsnewArray(): int {
    try {
        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const resObj = ExampleClass.instantiate(ESValue.wrap(v0), ESValue.wrap(v0))
        const resGetV0 = resObj.getProperty('v0')
        const resGetV1 = resObj.getProperty('v1')

        for (let i = 0; i < 10; ++i) {
            if (resGetV0.getProperty(i).toNumber() != v0[i] || resGetV1.getProperty(i).toNumber() != v0[i]) {
                return 1
            }
        }
        return 0
    } catch (e) {
        return 1
    }
}

function jsnewSetPropertyArray(): int {
    try {
        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const resObj = ClassWithEmptyConstructor.instantiate()

        resObj.setProperty('v0', ESValue.wrap(v0))
        resObj.setProperty('v1', ESValue.wrap(v0))
        const resGetV0 = resObj.getProperty('v0')
        const resGetV1 = resObj.getProperty('v1')

        for (let i = 0; i < 10; ++i) {
            if (resGetV0.getProperty(i).toNumber() != v0[i] || resGetV1.getProperty(i).toNumber() != v0[i]) {
                return 1
            }
        }
        return 0
    } catch (e) {
        return 1
    }
}

function jscallStaticMethodArray(): int {
    try {
        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const res = ExampleClass.invokeMethod('evaluateArray', ESValue.wrap(v0), ESValue.wrapNumber(10))
        for (let i = 0; i < 10; ++i) {
            if (res.getProperty(i).toNumber() != 2 * i * i) {
                return 1
            }
        }
        return 0
    } catch (e) {
        return 1
    }
}

function testNamespace(): int {
    try {
        const v0: int = 1
        const v1: String = 'Pinky'
        const resObj = MyNamespace.invokeMethod('createKitten', ESValue.wrapNumber(v0), ESValue.wrapString(v1))
        const resId = resObj.getProperty('id').toNumber()
        const resName = resObj.getProperty('name').toString()
        if (resId == v0 && resName == v1) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}