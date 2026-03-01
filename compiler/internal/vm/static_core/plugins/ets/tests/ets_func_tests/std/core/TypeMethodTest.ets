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

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name)
        return 0
    }
    console.println("FAILED: " + name)
    return 1
}

interface Serializable {
    serialize(): String;
}

class Point implements Serializable {
    protected x: double
    y: double

    override serialize(): String {
        return "{" + this.x + "," + this.y + "}"
    }

    add(oth: Point): Point {
        let res = new Point()
        res.x = this.x + oth.x
        res.y = this.y + oth.y
        return res
    }

    optionalParams(x: int, opt1: int = 1, opt2: int = 2) {}
    restParams(x: int[], ...rest: int[]) {}
    optReadonlyRestParams(x: readonly int[] = [4], y: int = 4, z: readonly [number, string] = [1, "123"], ...rest: int[]) {}
}

abstract class Writer {
    abstract write(): Int;
    protected close() {}
}

class HTMLWriter extends Writer {
    private state_: int

    get state(): int {
        return this.state_
    }

    private set state(state: int) {
        this.state_ = state;
    }

    static createFileExt(file: String): String {
        return file + ".html"
    }

    override write(): Int {
        return new Int(2)
    }

    constructor(state: int) {
        this.state_ = state
    }

    final finalMethod() {}

    native nativeMethod(): boolean

    async asyncMethod() {}
}

function findMethodByName(ct: ClassType, name: string): Method | null {
    for (let i = 0; i < ct.getMethodsNum(); ++i) {
        let method = ct.getMethod(i)
        if (method.getName() == name) {
            return method
        }
    }
    return null
}

function testGetReceiverTypeError(staticMethod: MethodType): boolean {
    try {
        staticMethod.getReceiverType()
    } catch (ignored) {
        if (ignored instanceof TypeError){
            return true
        }
        // Expected error is catched
    }
    return false
}

function testMethodType(): int {
    let pt = Type.of(new Point()) as ClassType
    let hwt = Type.of(new HTMLWriter(10)) as ClassType

    let serializeMethodType = findMethodByName(pt, "serialize")!.getType()
    let addMethodType = findMethodByName(pt, "add")!.getType()
    let cType = hwt.getConstructor(0).getType()
    let getterType = findMethodByName(hwt, "%%get-state")!.getType()
    let setterType = findMethodByName(hwt, "%%set-state")!.getType()

    return  test(serializeMethodType.getParametersNum() == 0,           "   0 Parameters") +
            test(serializeMethodType.getResultType() == StringType.REF, "   has String result type") +
            test(serializeMethodType.getReceiverType().equals(pt),      "   has Point receiver type") +
            test(addMethodType.getParametersNum() == 1,                 "   1 parameter:") +
            test(addMethodType.getParameter(0).getType().equals(pt),    "       Point parameter") +
            test(addMethodType.getResultType().equals(pt),              "   has Point result type") +
            test(addMethodType.getReceiverType().equals(pt),            "   has Point receiver type") +
            test(cType.getParametersNum() == 1,                         "   1 Parameter") +
            test(cType.getParameter(0).getType() == IntType.VAL,        "       int parameter") +
            test(cType.getResultType() == VoidType.VAL,                 "   has void result type") +
            test(cType.getReceiverType().equals(hwt),                   "   has HTMLWriter receiver type") +
            test(getterType.getParametersNum() == 0,                    "   0 Parameters") +
            test(getterType.getResultType() == IntType.VAL,             "   has int result type") +
            test(getterType.getReceiverType().equals(hwt),              "   has HTMLWriter receiver type") +
            test(setterType.getParametersNum() == 1,                    "   1 Parameter") +
            test(setterType.getParameter(0).getType() == IntType.VAL,   "       int parameter") +
            test(setterType.getResultType() == VoidType.VAL,            "   has void result type") +
            test(setterType.getReceiverType().equals(hwt),              "   has HTMLWriter receiver type")
}

function testMethod() {
    let pt = Type.of(new Point()) as ClassType
    let hwt = Type.of(new HTMLWriter(10)) as ClassType
    let wt = hwt.getBaseType()

    let cMethod = hwt.getConstructor(0)
    let getterMethod = findMethodByName(hwt, "%%get-state")!
    let setterMethod = findMethodByName(hwt, "%%set-state")!
    let createFileExtMethod = findMethodByName(hwt, "createFileExt")!
    let abstractWriteMethod = findMethodByName(wt, "write")!
    let overrideWriteMethod = findMethodByName(hwt, "write")!
    let closeMethod = findMethodByName(hwt, "close")!
    let finalMethod = findMethodByName(hwt, "finalMethod")!
    let nativeMethod = findMethodByName(hwt, "nativeMethod")!
    let asyncMethod = findMethodByName(hwt, "asyncMethod")!

    let isInherited = (m: Method, cls: ClassType): boolean => {
        return m.getOwnerType().getName() != cls.getName()
    };
    let isOverrided = (m: Method, cls: ClassType): boolean => {
        if (!m.getOwnerType().equals(cls)) {
            return false;
        }
        // should be ClassType API instead of function findMethodByName
        return findMethodByName(cls.getBaseType(), m.getName()) != undefined
    };

    return test(cMethod.getName() == "constructor",                            "MethodName: constructor") +
            test(cMethod.getAttributes() == Attributes.CONSTRUCTOR,             "   constructor attrib") +
            test(cMethod.getAccessModifier() == AccessModifier.PUBLIC,          "   public access modifier") +
            test(!isInherited(cMethod, hwt),                                    "   not inherited") +
            test(!isOverrided(cMethod, hwt),                                    "   not override") +
            test(!cMethod.isStatic(),                                           "   not static") +
            test(!cMethod.isFinal(),                                            "   not final") +
            test(!cMethod.isNative(),                                           "   not native") +
            test(!cMethod.isAsync(),                                            "   not async") +
            test(!cMethod.isAbstract(),                                         "   not abstract") +
            test(!cMethod.isGetter(),                                           "   not getter") +
            test(!cMethod.isSetter(),                                           "   not setter") +
            test(getterMethod.getName() == "%%get-state",                        "MethodName: %%get-state") +
            test(getterMethod.getAttributes() == Attributes.GETTER,             "   getter attrib") +
            test(getterMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!isInherited(getterMethod, hwt),                               "   not inherited") +
            test(!isOverrided(getterMethod, hwt),                               "   not override") +
            test(!getterMethod.isStatic(),                                      "   not static") +
            test(!getterMethod.isAbstract(),                                    "   not abstract") +
            test(getterMethod.isGetter(),                                       "   getter") +
            test(!getterMethod.isSetter(),                                      "   not setter") +
            test(!getterMethod.isFinal(),                                       "   not final") +
            test(!getterMethod.isNative(),                                      "   not native") +
            test(!getterMethod.isAsync(),                                       "   not async") +
            test(setterMethod.getName() == "%%set-state",                        "MethodName: %%set-state") +
            test(setterMethod.getAttributes() == Attributes.SETTER,             "   setter attrib") +
            test(setterMethod.getAccessModifier() == AccessModifier.PRIVATE,    "   private access modifier") +
            test(!isInherited(setterMethod, hwt),                               "   not inherited") +
            test(!isOverrided(setterMethod, hwt),                               "   not override") +
            test(!setterMethod.isStatic(),                                      "   not static") +
            test(!setterMethod.isAbstract(),                                    "   not abstract") +
            test(!setterMethod.isGetter(),                                      "   not getter") +
            test(setterMethod.isSetter(),                                       "   setter") +
            test(!setterMethod.isFinal(),                                       "   not final") +
            test(!setterMethod.isNative(),                                      "   not native") +
            test(!setterMethod.isAsync(),                                       "   not async") +
            test(createFileExtMethod.getName() == "createFileExt",                     "MethodName: createFileExt") +
            test(createFileExtMethod.getAttributes() == Attributes.STATIC,             "   static attrib") +
            test(createFileExtMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!isInherited(createFileExtMethod, hwt),                               "   not inherited") +
            test(!isOverrided(createFileExtMethod, hwt),                               "   not override") +
            test(createFileExtMethod.isStatic(),                                       "   static") +
            test(!createFileExtMethod.isAbstract(),                                    "   not abstract") +
            test(!createFileExtMethod.isGetter(),                                      "   not getter") +
            test(!createFileExtMethod.isSetter(),                                      "   not setter") +
            test(!createFileExtMethod.isFinal(),                                       "   not final") +
            test(!createFileExtMethod.isNative(),                                      "   not native") +
            test(!createFileExtMethod.isAsync(),                                       "   not async") +
            test(testGetReceiverTypeError(createFileExtMethod!.getType()),             "   has no receiver") +
            test(overrideWriteMethod.getName() == "write",                             "MethodName: write") +
            test(overrideWriteMethod.getAttributes() == 0,                             "   no attributes") +
            test(overrideWriteMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!isInherited(overrideWriteMethod, hwt),                               "   not inherited") +
            test(isOverrided(overrideWriteMethod, hwt),                                "   override") +
            test(!overrideWriteMethod.isStatic(),                                      "   not static") +
            test(!overrideWriteMethod.isAbstract(),                                    "   not abstract") +
            test(!overrideWriteMethod.isGetter(),                                      "   not getter") +
            test(!overrideWriteMethod.isSetter(),                                      "   not setter") +
            test(!overrideWriteMethod.isFinal(),                                       "   not final") +
            test(!overrideWriteMethod.isNative(),                                      "   not native") +
            test(!overrideWriteMethod.isAsync(),                                       "   not async") +
            test(abstractWriteMethod.getName() == "write",                             "MethodName: write") +
            test(abstractWriteMethod.getAttributes() == Attributes.ABSTRACT,           "   abstract attrib") +
            test(abstractWriteMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!isInherited(abstractWriteMethod, wt),                                "   not inherited") +
            test(!isOverrided(abstractWriteMethod, wt),                                "   not override") +
            test(!abstractWriteMethod.isStatic(),                                      "   not static") +
            test(abstractWriteMethod.isAbstract(),                                     "   abstract") +
            test(!abstractWriteMethod.isGetter(),                                      "   not getter") +
            test(!abstractWriteMethod.isSetter(),                                      "   not setter") +
            test(!abstractWriteMethod.isFinal(),                                       "   not final") +
            test(!abstractWriteMethod.isNative(),                                      "   not native") +
            test(!abstractWriteMethod.isAsync(),                                       "   not async") +
            test(closeMethod.getName() == "close",                             "MethodName: close") +
            test(closeMethod.getAttributes() == 0,                             "   no attributes") +
            test(closeMethod.getAccessModifier() == AccessModifier.PROTECTED,  "   protected access modifier") +
            test(isInherited(closeMethod, hwt),                                "   inherited") +
            test(!isOverrided(closeMethod, hwt),                               "   not override") +
            test(!closeMethod.isStatic(),                                      "   not static") +
            test(!closeMethod.isAbstract(),                                    "   not abstract") +
            test(!closeMethod.isGetter(),                                      "   not getter") +
            test(!closeMethod.isSetter(),                                      "   not setter") +
            test(!closeMethod.isFinal(),                                       "   not final") +
            test(!closeMethod.isNative(),                                      "   not native") +
            test(!closeMethod.isAsync(),                                       "   not async") +
            test(finalMethod.getName() == "finalMethod",                       "MethodName: finalMethod") +
            test(finalMethod.getAttributes() == Attributes.FINAL,              "   final attrib") +
            test(finalMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!isInherited(finalMethod, hwt),                               "   not inherited") +
            test(!isOverrided(finalMethod, hwt),                               "   not override") +
            test(!finalMethod.isStatic(),                                      "   not static") +
            test(!finalMethod.isAbstract(),                                    "   not abstract") +
            test(!finalMethod.isGetter(),                                      "   not getter") +
            test(!finalMethod.isSetter(),                                      "   not setter") +
            test(finalMethod.isFinal(),                                        "   final") +
            test(!finalMethod.isNative(),                                      "   not native") +
            test(!finalMethod.isAsync(),                                       "   not async") +
            test(nativeMethod.getName() == "nativeMethod",                     "MethodName: nativeMethod") +
            test(nativeMethod.getAttributes() == Attributes.NATIVE,            "   native attrib") +
            test(nativeMethod.getAccessModifier() == AccessModifier.PUBLIC,    "   public access modifier") +
            test(!isInherited(nativeMethod, hwt),                              "   not inherited") +
            test(!isOverrided(nativeMethod, hwt),                              "   not override") +
            test(!nativeMethod.isStatic(),                                     "   not static") +
            test(!nativeMethod.isAbstract(),                                   "   not abstract") +
            test(!nativeMethod.isGetter(),                                     "   not getter") +
            test(!nativeMethod.isSetter(),                                     "   not setter") +
            test(!nativeMethod.isFinal(),                                      "   not final") +
            test(nativeMethod.isNative(),                                      "   native") +
            test(!nativeMethod.isAsync(),                                      "   not async") +
            test(asyncMethod.getName() == "asyncMethod",                       "MethodName: asyncMethod") +
            test(asyncMethod.getAttributes() == Attributes.ASYNC,              "   async attrib") +
            test(asyncMethod.getAccessModifier() == AccessModifier.PUBLIC,     "   public access modifier") +
            test(!isInherited(asyncMethod, hwt),                               "   not inherited") +
            test(!isOverrided(asyncMethod, hwt),                               "   not override") +
            test(!asyncMethod.isStatic(),                                      "   not static") +
            test(!asyncMethod.isAbstract(),                                    "   not abstract") +
            test(!asyncMethod.isGetter(),                                      "   not getter") +
            test(!asyncMethod.isSetter(),                                      "   not setter") +
            test(!asyncMethod.isFinal(),                                       "   not final") +
            test(!asyncMethod.isNative(),                                      "   not native") +
            test(asyncMethod.isAsync(),                                      "   async")
}

function testParameters() {
    let pt = Type.of(new Point()) as ClassType

    let optionalParamsMethod = findMethodByName(pt, "optionalParams")!.getType()
    let restParamsMethod = findMethodByName(pt, "restParams")!.getType()
    let optReadonlyRestParamsMethod = findMethodByName(pt, "optReadonlyRestParams")!.getType()

    return  test(optionalParamsMethod.getParametersNum() == 3,          "   3 Parameters") +
            test(!optionalParamsMethod.getParameter(0).isOptional(),    "   param0 is not optional") +
            test(optionalParamsMethod.getParameter(1).isOptional(),     "   param1 is optional") +
            test(optionalParamsMethod.getParameter(2).isOptional(),     "   param2 is optional") +
            test(restParamsMethod.getParametersNum() == 2,              "   3 Parameters") +
            test(!restParamsMethod.getParameter(0).isRest(),            "   param0 is not rest") +
            test(restParamsMethod.getParameter(1).isRest(),             "   param1 is rest") +
            test(optReadonlyRestParamsMethod.getParametersNum() == 4,         "   3 Parameters") +
            test(optReadonlyRestParamsMethod.getParameter(0).isOptional(),    "   param0 is optional") +
            test(!optReadonlyRestParamsMethod.getParameter(0).isRest(),       "   param0 is not rest") +
            test(optReadonlyRestParamsMethod.getParameter(1).isOptional(),    "   param1 is optional") +
            test(!optReadonlyRestParamsMethod.getParameter(1).isRest(),       "   param1 is not rest") +
            test(optReadonlyRestParamsMethod.getParameter(2).isOptional(),    "   param2 is optional") +
            test(!optReadonlyRestParamsMethod.getParameter(2).isRest(),       "   param2 is not rest") +
            test(!optReadonlyRestParamsMethod.getParameter(3).isOptional(),   "   param3 is not optional") +
            test(optReadonlyRestParamsMethod.getParameter(3).isRest(),        "   param3 is rest")
}

function main(): int {
    let failures = 0
    failures += testMethodType()
    failures += testMethod()
    failures += testParameters()
    if (failures == 0) {
        console.println("PASSED: All tests run")
    } else {
        console.println("FAILED: All tests run")
    }
    return failures
}
