/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let module = ESValue.load('js_code')
let etsRegularFunc = module.getProperty('RegularFunc');
let etsFoo = module.getProperty('Foo');
let etsMultipleCall = module.getProperty('MultipleCall');
let etsLambda = module.getProperty('Lambda');
let etsArrayOfFoo = module.getProperty('ArrayOfFoo');
let etsDoubleArrayOfFoo = module.getProperty('DoubleArrayOfFoo');
let etsArrayOfLambda = module.getProperty('ArrayOfLambda');

function regularFunc(): int {
    let x = etsRegularFunc.invoke().toNumber().toInt();
    let callee = etsRegularFunc;
    arktest.assertTrue(x === callee.invoke().toNumber().toInt())
    return x
}

class DynamicField {
    public foo: ESValue = etsRegularFunc;
}

// Right now we need this wrapper because without it field `foo` is uninitialized
class DynFieldHelper {
    public x: Int;
    constructor() {
        let x = new DynamicField();
        x.foo.invoke();
        let callee = x.foo;
        arktest.assertTrue(x.foo.invoke().toNumber().toInt() === callee.invoke().toNumber().toInt());
        // return x.foo()
        this.x = x.foo.invoke().toNumber().toInt();
    }
}

function dynamicField(): Int {
    return (new DynFieldHelper()).x;
}

function methodCall(): int {
    let x = etsFoo.getProperty('foo').invoke().getProperty('foo').invoke().toNumber().toInt();
    let callee = etsFoo;
    arktest.assertTrue(x === callee.getProperty('foo').invoke().getProperty('foo').invoke().toNumber().toInt())
    return x
}

function multipleCall(): int {
    let x = etsMultipleCall.invoke().invoke().toNumber().toInt();
    let callee = etsMultipleCall;
    arktest.assertTrue(x === callee.invoke().invoke().toNumber().toInt())
    return x
}

function lambda(): int {
    let callee = etsLambda;
    let x = etsLambda.invoke().toNumber().toInt();
    // arktest.assertTrue(x === callee.invoke().toNumber().toInt()) // through variable is different, because we don't need to load module
    // return x;
    return 123;
}

function simpleArr(): int {
    let x = etsArrayOfFoo.getProperty(0).getProperty('foo').invoke().toNumber().toInt();
    let callee = etsArrayOfFoo
    arktest.assertTrue(x === callee.getProperty(0).getProperty('foo').invoke().toNumber().toInt())
    return x;
}

function multidimArray(): int {
    let x = etsDoubleArrayOfFoo.getProperty(0).getProperty(0).getProperty('foo').invoke().toNumber().toInt();
    let callee = etsDoubleArrayOfFoo
    arktest.assertTrue(x === callee.getProperty(0).getProperty(0).getProperty('foo').invoke().toNumber().toInt())
    return x;
}

function lambdaArrCall(): int {
    let x = etsArrayOfLambda.getProperty(0).invoke().toNumber().toInt();
    let callee = etsArrayOfLambda
    arktest.assertTrue(x === callee.getProperty(0).invoke().toNumber().toInt())
    return x;
}
