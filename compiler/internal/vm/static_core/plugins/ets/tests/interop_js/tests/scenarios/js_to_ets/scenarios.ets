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

let module = ESValue.load('pure_js');
let standaloneFunctionJs = module.getProperty('standaloneFunctionJs');
let ClassWithMethodJs = module.getProperty('ClassWithMethodJs');
let newInterfaceWithMethod = module.getProperty('newInterfaceWithMethod');
let ClassWithGetterSetter = module.getProperty('ClassWithGetterSetter');
let lambdaFunction = module.getProperty('lambdaFunction');
let genericFunction = module.getProperty('genericFunction');
let functionArgTypeAny = module.getProperty('functionArgTypeAny');
let functionArgTypeUnknown = module.getProperty('functionArgTypeUnknown');
let functionArgTypeUndefined = module.getProperty('functionArgTypeUndefined');
let functionArgTypeTuple = module.getProperty('functionArgTypeTuple');
let functionReturnTypeAny = module.getProperty('functionReturnTypeAny');
let functionReturnTypeUnknown = module.getProperty('functionReturnTypeUnknown');
let functionReturnTypeUndefined = module.getProperty('functionReturnTypeUndefined');
let functionDefaultIntParameterFunction = module.getProperty('functionDefaultIntParameterFunction');
let functionDefaultParameterFunction = module.getProperty('functionDefaultParameterFunction');
let genericTypeParameter = module.getProperty('genericTypeParameter');
let genericTypeReturnValue = module.getProperty('genericTypeReturnValue');
let functionArgTypeOptionalPrimitive = module.getProperty('functionArgTypeOptionalPrimitive');
let functionArgTypePrimitive = module.getProperty('functionArgTypePrimitive');
let functionReturnTypePrimitive = module.getProperty('functionReturnTypePrimitive');
let jsSumRestParams = module.getProperty('jsSumRestParams');
let jsMultiplyArgBySumRestParams = module.getProperty('jsMultiplyArgBySumRestParams');
let jsMultiplySumArgsBySumRestParams = module.getProperty('jsMultiplySumArgsBySumRestParams');
let jsConcatStringsRestParams = module.getProperty('jsConcatStringsRestParams');
let functionArgTypeUnion = module.getProperty('functionArgTypeUnion');
let functionReturnTypeUnion = module.getProperty('functionReturnTypeUnion');
let UnionTestClassJs = module.getProperty('UnionTestClassJs');
let newInterfaceWithUnion = module.getProperty('newInterfaceWithUnion');
let ClassWithStaticMethod = module.getProperty('ClassWithStaticMethod');
let ClassReturnThis = module.getProperty('ClassReturnThis');
let functionReturnOmittedValue = module.getProperty('functionReturnOmittedValue');
let functionRestParameter = module.getProperty('functionRestParameter');
let functionOverload = module.getProperty('functionOverload');
let functionCallableReturnValue = module.getProperty('functionCallableReturnValue');
let functionArgStringLiteralType = module.getProperty('functionArgStringLiteralType');
let functionIntersectionTypePrimitive = module.getProperty('functionIntersectionTypePrimitive');
let ClassWithInFieldDeclaration = module.getProperty('ClassWithInFieldDeclaration');
let functionReturnsCompositeType = module.getProperty('functionReturnsCompositeType');

function testOptionals(): boolean {
    // #22991
    // let x = optionalCall(123, 321, 5)
    // arktest.assertTrue(x == 123 + 321 + 5);
    // x = optionalCall(23, 21)
    // arktest.assertTrue(x == 23 + 21 + 1);
    // x = optionalCall(100)
    // arktest.assertTrue(x == 100 + 130 + 1);
    // x = optionalCall()
    // arktest.assertTrue(x == 123 + 130 + 1);

    // #22991
    // x = singleRequired(123, 321)
    // arktest.assertTrue(x == 123 + 321 + 123);
    // x = singleRequired(100)
    // arktest.assertTrue(x == 100 + 123 + 123);

    return true;
}

function TestStandaloneFunctionCall(): boolean {
    return standaloneFunctionJs.invoke().toNumber() === 1;
}

function TestClassMethodCall(): boolean {
    let classWithMethodJs = ClassWithMethodJs.instantiate();
    return classWithMethodJs.getProperty('methodInClassJs').invoke().toNumber() === 1;
}

function TestInterfaceMethodCall(): boolean {
    let interfaceMethod = newInterfaceWithMethod.invoke().getProperty('methodInInterface');
    return interfaceMethod.invoke().toNumber() === 1;
}

function TestClassGetter(): boolean {
    return ClassWithGetterSetter.instantiate().getProperty('value').toNumber() === 1;
}

function TestClassSetter(): boolean {
    let classWithGetterSetter = ClassWithGetterSetter.instantiate();
    classWithGetterSetter.setProperty('value', 2);
    return classWithGetterSetter.getProperty('value').toNumber() === 2;
}

function TestLambdaFunctionCall(): boolean {
    return lambdaFunction.invoke().toNumber() === 1;
}

function TestGenericFunctionCall(): boolean {
    return genericFunction.invoke(1).toNumber() === 1;
}

class ClassToExtend1 {
    value(): int {
        return 1;
    }
}

// NOTE(oignatenko) uncomment below after interop is implemented in this direction
// class Child extends ClassToExtend {}
function TestExtendClass(): boolean {
    // return (new ClassToExtend()).value().toNumber() === 1;
    return false;
}

function TestFunctionArgTypeAny(): boolean {
    let arg: Any = 1;
    return functionArgTypeAny.invoke(arg).toNumber() === 1;
}

function TestFunctionArgTypeUnknown(): boolean {
    let arg: Any = 1;
    return functionArgTypeUnknown.invoke(arg).toNumber() === 1;
}

function TestFunctionArgTypeUndefined(): boolean {
    let arg: Any = 1;
    return functionArgTypeUndefined.invoke(arg).toNumber() === 1;
}

function TestFunctionArgTypeTuple(): boolean {
    let arg: [int, String] = [1, "one"];
    return functionArgTypeTuple.invoke(arg).toNumber() === 1;
}

function TestFunctionReturnTypeAny(): boolean {
    return functionReturnTypeAny.invoke().toNumber() === 1;
}

function TestFunctionReturnTypeUnknown(): boolean {
    return functionReturnTypeUnknown.invoke().toNumber() === 1;
}

function TestFunctionReturnTypeUndefined(): boolean {
    return functionReturnTypeUndefined.invoke().toNumber() === 1;
}

function callable(): int {
    return 1;
}

function TestFunctionArgTypeCallable(): boolean {
    return false;
    // NOTE #1646 extract this to separate ets file
    // NOTE #15891 uncomment below after interop is implemented in this direction
    // return functionArgTypeCallable(callable).toNumber() === 1;
}

function TestDefaultValueIntDefineForParameter(): boolean {
    return functionDefaultIntParameterFunction.invoke().toNumber() === 1;
}

function TestDefaultValueDefineForParameter(): boolean {
    return functionDefaultParameterFunction.invoke().toNumber() === 1;
}

function TestGenericTypeAsParameter(): boolean {
    return genericTypeParameter.invoke(1).toString() === "1";
}

function TestGenericTypeAsReturnValue(): boolean {
    return genericTypeReturnValue.invoke(1).toNumber() === 1;
}

function TestFunctionArgTypeOptionalPrimitiveExplicit(): boolean {
    return functionArgTypeOptionalPrimitive.invoke(1).toNumber() === 1;
}

function TestFunctionArgTypeOptionalPrimitiveDefault(): boolean {
    return functionArgTypeOptionalPrimitive.invoke().toNumber() === 1;
}

function TestFunctionArgTypePrimitive(): boolean {
    let arg: int = 1;
    return functionArgTypePrimitive.invoke(arg).toNumber() === 1;
}

function TestFunctionReturnTypePrimitive(): boolean {
    let res: boolean = functionReturnTypePrimitive.invoke().toBoolean();
    return typeof res === "boolean";
}

function TestRestParams() {
    // NOTE: casts due to bug in frontend
    arktest.assertEQ(jsSumRestParams.invoke(1, 2, 3).toNumber(), (1 + 2 + 3) );
    arktest.assertEQ(jsMultiplyArgBySumRestParams.invoke(1, 2, 3, 4).toNumber(), (1) * (2 + 3 + 4));
    arktest.assertEQ(jsMultiplySumArgsBySumRestParams.invoke(1, 2, 3, 4, 5).toNumber(), (1 + 2) * (3 + 4 + 5));
    arktest.assertEQ(jsConcatStringsRestParams.invoke().toString(), "");
    arktest.assertEQ(jsConcatStringsRestParams.invoke("a", "b", "c").toString(), "abc");
    return true;
}

function TestFunctionArgTypeUnion(): boolean {
    let i: (number | string) = 1;
    let s: (number | string) = "1";
    if (functionArgTypeUnion.invoke(i).toNumber() !== 0) {
        return false;
    }
    if (functionArgTypeUnion.invoke(s).toNumber() !== 1) {
        return false;
    }
    return true;
}

function TestFunctionReturnTypeUnion(): boolean {
    let res: (number | string) = functionReturnTypeUnion.invoke(0).unwrap() as (number | string);
    if (typeof res !== "number" || res as number !== 1) {
        return false;
    }
    res = functionReturnTypeUnion.invoke(1).unwrap() as (number | string);
    if (typeof res !== "string" || res.toString() !== "1") {
        return false;
    }
    return true;
}

function TestClassMethodArgTypeUnion(): boolean {
    let i: (number | string) = 1;
    let s: (number | string) = "1";
    let obj = UnionTestClassJs.instantiate();
    if (obj.invokeMethod('methodArgTypeUnion', i).toNumber() !== 0) {
        return false;
    }
    if (obj.invokeMethod('methodArgTypeUnion', s).toNumber() !== 1) {
        return false;
    }
    return true;
}

function TestClassMethodReturnTypeUnion(): boolean {
    let obj = UnionTestClassJs.instantiate();
    let res: (number | string) = obj.invokeMethod('methodReturnTypeUnion', 0).unwrap() as (number | string);
    if (typeof res !== "number" || res as number !== 1) {
        return false;
    }
    res = obj.invokeMethod('methodReturnTypeUnion', 1).unwrap() as (number | string);
    if (typeof res !== "string" || res.toString() !== "1") {
        return false;
    }
    return true;
}

function TestInterfaceMethodCallReturnTypeUnion(): boolean {
    let obj = newInterfaceWithUnion.instantiate();
    let res: (number | string) = obj.invokeMethod('methodInInterface', 0).unwrap() as (number | string);
    if (typeof res !== "number" || res as number !== 1) {
        return false;
    }
    res = obj.invokeMethod('methodInInterface', 1).unwrap() as (number | string);
    if (typeof res !== "string" || res.toString() !== "1") {
        return false;
    }
    return true;
}

function TestStaticMethodCall(): boolean {
    return ClassWithStaticMethod.invokeMethod('staticMethod', 1).toNumber() === 1;
}

function TestStaticMethodCallReturnTypeUnion(): boolean {
    let resObj: ESValue = ClassWithStaticMethod.invokeMethod('staticMethodReturnTypeUnion', 0);
    let res: (number | string) = resObj.unwrap() as (number | string);
    if (typeof res !== "number" || res as number !== 1) {
        return false;
    }
    resObj = ClassWithStaticMethod.invokeMethod('staticMethodReturnTypeUnion', 1);
    res = resObj.unwrap() as (number | string);
    if (typeof res !== "string" || res.toString() !== "1") {
        return false;
    }
    return true;
}

function testReturnIsThis(): boolean {
    let obj = ClassReturnThis.instantiate();
    return obj.invokeMethod('returnThis') === obj;
}

function testReturnIsOmitted(): boolean {
    return functionReturnOmittedValue.invoke().unwrap() as Object | undefined === undefined;
}

function testFunctionRestParameter(): boolean {
    return functionRestParameter.invoke(1, 2, 3).toNumber() === 1;
}

function Test_function_spread_parameter(): boolean {
    // NOTE(nikitayegorov, DZ) uncomment 2 lines below after rest\spread is fixed (#17339)
    // const arr = [1, 1];
    // return functionSpreadParameter(...arr).toNumber() === 2;
    return false;
}

function testFunctionOverload(): boolean {
    return functionOverload.invoke().toNumber() === 1;
}

function testFunctionCallableReturnValue(): boolean {
    const fn = functionCallableReturnValue;
    return fn.invoke().invoke(1).toNumber() === 2;
}

function testFunctionArgStringLiteralType(): boolean {
    let arg: Any = 1;
    return functionArgStringLiteralType.invoke(arg).toNumber() === 1;
}

function testFunctionIntersectionTypePrimitive(): boolean {
    let arg: Any = 1;
    return functionIntersectionTypePrimitive.invoke(arg).toNumber() === 1;
}

function testClassInPlaceFieldDeclarations(): boolean {
    let obj = ClassWithInFieldDeclaration.instantiate(1);
    return obj.getProperty('value').toNumber() === 1;
}

function testFunctionReturnsCompositeTypeInt(): boolean {
    let isOk = true;
    for (let i = 0; i < 4; i++){
        const expectedValue = functionReturnsCompositeType.invoke(i).unwrap() as (string | number | double | Array<string>);
        switch (i) {
            case 0:
                isOk = isOk && expectedValue as String === '1';
                break;
            case 1:
                isOk = isOk &&  expectedValue as double === 1.0;
                break;
            case 2:
                isOk = isOk &&  expectedValue as number === 1;
                break;
            default:
                isOk = isOk && typeof expectedValue === 'object';
                break;
        }
    }
    return isOk;
}
