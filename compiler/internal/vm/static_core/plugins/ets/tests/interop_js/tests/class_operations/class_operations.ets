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

class C {
    hash: int
    val: ESValue

    constructor(hash: int, val: ESValue) {
        this.hash = hash
        this.val = val
    }
}

function testJSONStringifyESObjectInstance(): boolean {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const moduleObj = gtestEnv.getProperty("module")
        const createObjFn = moduleObj.getProperty("MyNamespace").getProperty("createKitten")

        const id: number = 1.0
        const name: string = "Nora"
        const esObj = createObjFn.invoke(ESValue.wrapNumber(id), ESValue.wrapString(name))

        const unwrappedObj = esObj.unwrap()
        arktest.assertEQ(JSON.stringify(unwrappedObj), `{"id":1,"name":"Nora","male":false}`)

        esObj.setProperty("self", esObj)
        let hasCircularError = false
        try {
            JSON.stringify(esObj.unwrap())
        } catch (e) {
            hasCircularError = true
        }
        
        if (!hasCircularError) {
            return false
        }

        esObj.setProperty("self", ESValue.instantiateEmptyObject())

        const esObjName = esObj.getProperty("name")
        arktest.assertEQ(JSON.stringify(esObjName.unwrap()), '"Nora"')

        const withFnObj = ESValue.instantiateEmptyObject()
        withFnObj.setProperty("name", ESValue.wrapString("foo"))

        const moduleFn = moduleObj.getProperty("doNothing")
        withFnObj.setProperty("fn", moduleFn)

        arktest.assertEQ(JSON.stringify(withFnObj.unwrap()), `{"name":"foo"}`)

        const c = new C(42, esObj)
        const valJson = JSON.stringify(esObj.unwrap())
        const expectedJson = `{"hash":42,"val":${valJson}}`
        const cJson = `{"hash":${c.hash},"val":${valJson}}`
        arktest.assertEQ(cJson, expectedJson)

        return true
    } catch (e) {
        return false
    }
}

function testJSONStringifyESObjectArray(): boolean {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const moduleObj = gtestEnv.getProperty("module")

        const arr: FixedArray<number> = new FixedArray<number>(3)
        for (let i = 0; i < arr.length; i++) {
            arr[i] = i + 1
        }

        const evalArrFn = moduleObj.getProperty("evaluateArray")
        const arrEsObj = evalArrFn.invoke(ESValue.wrap(arr), ESValue.wrapNumber(arr.length))

        arktest.assertEQ(JSON.stringify(arrEsObj.unwrap()), "[1,3,7]")

        arrEsObj.setProperty(3, arrEsObj)
        arktest.expectError(() => {
            JSON.stringify(arrEsObj.unwrap())
        }, "cyclic object value")

        const nestedArrEsObj = ESValue.instantiateEmptyArray()
        nestedArrEsObj.setProperty(0, ESValue.wrapString("foo"))

        arrEsObj.setProperty(3, nestedArrEsObj)
        arktest.assertEQ(JSON.stringify(arrEsObj.unwrap()), `[1,3,7,["foo"]]`)

        return true
    } catch (e) {
        return false
    }
}

function jscallEmpty(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        module.invokeMethod("evaluateNumber")
        return 0
    } catch (e) {
        return 1
    }
}

function jsnewEmpty(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ClassWithEmptyConstructor = module.getProperty("ClassWithEmptyConstructor")
        const resObj = ClassWithEmptyConstructor.instantiate()

        const resGetV0 = resObj.getProperty("v0").toNumber()
        const resGetV1 = resObj.getProperty("v1").toNumber()

        if (resGetV0 == 42 && resGetV1 == 42) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jscallStaticMethodEmpty(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ExampleClass = module.getProperty("ExampleClass")
        ExampleClass.invokeMethod("emptyMethod")
        return 0
    } catch (e) {
        return 1
    }
}

function jscallObject(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ExampleClass = module.getProperty("ExampleClass")
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))

        const res = module.invokeMethod("evaluateObject", resObj)
        const resGetV0 = res.getProperty("v0").toNumber()
        const resGetV1 = res.getProperty("v1").toNumber()

        if (resGetV0 == 84 && resGetV1 == 84) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jsnewObject(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ExampleClass = module.getProperty("ExampleClass")
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))
        const res = ExampleClass.instantiate(resObj, resObj)

        const resGetV0 = res.getProperty("v0")
        const resGetV1 = res.getProperty("v1")

        if (resGetV0.getProperty("v0").toNumber() == 42 && resGetV0.getProperty("v1").toNumber() == 42 &&
            resGetV1.getProperty("v0").toNumber() == 42 && resGetV1.getProperty("v1").toNumber() == 42) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jsnewSetPropertyObject(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ClassWithEmptyConstructor = module.getProperty("ClassWithEmptyConstructor")
        const resObj = ClassWithEmptyConstructor.instantiate()
        const res = ClassWithEmptyConstructor.instantiate()

        resObj.setProperty("v0", res)
        resObj.setProperty("v1", res)

        const resGetV0 = resObj.getProperty("v0")
        const resGetV1 = resObj.getProperty("v1")

        if (resGetV0.getProperty("v0").toNumber() == 42 && resGetV0.getProperty("v1").toNumber() == 42 &&
            resGetV1.getProperty("v0").toNumber() == 42 && resGetV1.getProperty("v1").toNumber() == 42) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jscallMethodObject(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ExampleClass = module.getProperty("ExampleClass")
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))

        const res = module.invokeMethod("evaluateObject", resObj)
        const resGetV0 = res.getProperty("v0").toNumber()
        const resGetV1 = res.getProperty("v1").toNumber()

        if (resGetV0 == 84 && resGetV1 == 84) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jscallMethodSimple(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const ExampleClass = module.getProperty("ExampleClass")
        const resObj = ExampleClass.instantiate(ESValue.wrapNumber(42), ESValue.wrapNumber(42))

        const res = resObj.invokeMethod("instanceEvaluateNumber").toNumber()

        if (res == 84) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jscallString(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: String = "hello"
        const v1: String = "world"
        const res = module.invokeMethod("evaluateNumber", ESValue.wrapString(v0), ESValue.wrapString(v1)).toString()

        if (res == "helloworld") {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jsnewString(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: String = "hello"
        const v1: String = "world"
        
        const ExampleClass = module.getProperty("ExampleClass")
        const resObj = ExampleClass.instantiate(ESValue.wrapString(v0), ESValue.wrapString(v1))

        const resGetV0 = resObj.getProperty("v0").toString()
        const resGetV1 = resObj.getProperty("v1").toString()

        if (resGetV0 == v0 && resGetV1 == v1) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jsnewSetPropertyString(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: String = "hello"
        const v1: String = "world"

        const ClassWithEmptyConstructor = module.getProperty("ClassWithEmptyConstructor")
        const resObj = ClassWithEmptyConstructor.instantiate()

        resObj.setProperty("v0", ESValue.wrapString(v0))
        resObj.setProperty("v1", ESValue.wrapString(v1))

        const resGetV0 = resObj.getProperty("v0").toString()
        const resGetV1 = resObj.getProperty("v1").toString()

        if (resGetV0 == v0 && resGetV1 == v1) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}

function jscallStaticMethodString(): int {
    try {
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: String = "hello"
        const v1: String = "world"

        const ExampleClass = module.getProperty("ExampleClass")
        const res = ExampleClass.invokeMethod("evaluateNumber", ESValue.wrapString(v0), ESValue.wrapString(v1)).toString()

        if (res == "helloworld") {
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
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const res = module.invokeMethod("evaluateArray", ESValue.wrap(v0), ESValue.wrapNumber(10))

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
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const ExampleClass = module.getProperty("ExampleClass")
        const resObj = ExampleClass.instantiate(ESValue.wrap(v0), ESValue.wrap(v0))

        const resGetV0 = resObj.getProperty("v0")
        const resGetV1 = resObj.getProperty("v1")

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
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const ClassWithEmptyConstructor = module.getProperty("ClassWithEmptyConstructor")
        const resObj = ClassWithEmptyConstructor.instantiate()

        resObj.setProperty("v0", ESValue.wrap(v0))
        resObj.setProperty("v1", ESValue.wrap(v0))

        const resGetV0 = resObj.getProperty("v0")
        const resGetV1 = resObj.getProperty("v1")

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
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: FixedArray<int> = new FixedArray<int>(10)
        for (let i = 0; i < 10; ++i) {
            v0[i] = i * i
        }

        const ExampleClass = module.getProperty("ExampleClass")
        const res = ExampleClass.invokeMethod("evaluateArray", ESValue.wrap(v0), ESValue.wrapNumber(10))

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
        const global = ESValue.getGlobal()
        const gtestEnv = global.getProperty("gtest_env")
        const module = gtestEnv.getProperty("module")

        const v0: int = 1
        const v1: String = "Pinky"

        const resObj = module.getProperty("MyNamespace").invokeMethod("createKitten", ESValue.wrapNumber(v0), ESValue.wrapString(v1))
        const resId = resObj.getProperty("id").toNumber()
        const resName = resObj.getProperty("name").toString()

        if (resId == v0 && resName == v1) {
            return 0
        } else {
            return 1
        }
    } catch (e) {
        return 1
    }
}