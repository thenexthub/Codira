/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

import { myErrorInstance, MyError } from "testDecl";
import { fooInstance, fooErrorInstance } from "testDecl";
import { Foo, FooError } from "testDecl";

function testXrefObjectAll() {
    testXrefObjectGetPropertyByName();
    testXrefObjectGetPropertyByIndex();
    testXrefObjectGetPropertyByValue();

    testXrefObjectSetPropertyByName();
    testXrefObjectSetPropertyByIndex();
    testXrefObjectSetPropertyByValue();

    testXrefObjectIsInstanceOf();
    testXrefObjectInvokeMethod();

    testXrefObjectInvoke();

    testXrefHasProperty();

    testXrefObjectInstantiate();

    testXrefObjectStrictlyEqual();

    testXrefObjectIsTrue();

    testXrefObjectTypeof();
}

function testXrefObjectGetPropertyByName() {
    // test for XrefObject is an Mimic Proxy
    let fooErrorInstanceESValue = ESValue.wrap(fooErrorInstance);
    let barPropertyError = fooErrorInstanceESValue.getProperty("barProperty").toNumber();
    arktest.assertEQ(barPropertyError, 0x55aa);

    // test for XrefObject is an JSValue
    let fooInstanceESValue = ESValue.wrap(fooInstance);
    let barProperty = fooInstanceESValue.getProperty("barProperty").toNumber();
    arktest.assertEQ(barProperty, 0x55aa);
}

function testXrefObjectGetPropertyByIndex() {
    // test for XrefObject is an Mimic Proxy
    let arrayMimicESValue = ESValue.wrap(fooErrorInstance.arrayProperty);
    arktest.assertEQ(arrayMimicESValue.getProperty(0).toNumber(), 0);
    arktest.assertEQ(arrayMimicESValue.getProperty(1).toNumber(), 1);
    arktest.assertEQ(arrayMimicESValue.getProperty(2).toNumber(), 2);
    arktest.assertEQ(arrayMimicESValue.getProperty(3).toNumber(), 3);
    arktest.assertEQ(arrayMimicESValue.getProperty(4).toNumber(), 4);
    arktest.assertEQ(arrayMimicESValue.getProperty(5).unwrap(), undefined);

    // test for XrefObject is an JSValue
    let arrayLikeJSValueESValue = ESValue.wrap(fooInstance.arrayLikeProperty);
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(0).toNumber(), 0);
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(1).toNumber(), 1);
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(2).toNumber(), 2);
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(3).toNumber(), 3);
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(4).toNumber(), 4);
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(5).unwrap(), undefined);
}

function testXrefObjectGetPropertyByValue() {
    // test for XrefObject is an Mimic Proxy
    let fooErrorInstanceESValue = ESValue.wrap(fooErrorInstance);
    let barPropertyError = fooErrorInstanceESValue.getProperty(ESValue.wrap("barProperty")).toNumber();
    arktest.assertEQ(barPropertyError, 0x55aa);

    // test for XrefObject is an JSValue
    let fooInstanceESValue = ESValue.wrap(fooInstance);
    let barProperty = fooInstanceESValue.getProperty(ESValue.wrap("barProperty")).toNumber();
    arktest.assertEQ(barProperty, 0x55aa);
}

function testXrefObjectSetPropertyByName() {
    let setValue = Math.floor(Math.random() * 10000);

    // test for XrefObject is an Mimic Proxy
    let fooErrorInstanceESValue = ESValue.wrap(fooErrorInstance);
    fooErrorInstanceESValue.setProperty("barProperty", ESValue.wrap(setValue));
    arktest.assertEQ(fooErrorInstanceESValue.getProperty("barProperty").toNumber(), setValue);

    // test for XrefObject is an JSValue
    let fooInstanceESValue = ESValue.wrap(fooInstance);
    fooInstanceESValue.setProperty("barProperty", ESValue.wrap(setValue));
    arktest.assertEQ(fooInstanceESValue.getProperty("barProperty").toNumber(), setValue);
}

function testXrefObjectSetPropertyByIndex() {
    let setValue = Math.floor(Math.random() * 10000);

    // test for XrefObject is an Mimic Proxy
    let arrayMimicESValue = ESValue.wrap(fooErrorInstance.arrayProperty);
    arrayMimicESValue.setProperty(0, ESValue.wrap(setValue));
    arktest.assertEQ(arrayMimicESValue.getProperty(0).toNumber(), setValue);

    // test for XrefObject is an JSValue
    let arrayLikeJSValueESValue = ESValue.wrap(fooInstance.arrayLikeProperty);
    arrayLikeJSValueESValue.setProperty(0, ESValue.wrap(setValue));
    arktest.assertEQ(arrayLikeJSValueESValue.getProperty(0).toNumber(), setValue);
}

function testXrefObjectSetPropertyByValue() {
    let setValue = Math.floor(Math.random() * 10000);

    // test for XrefObject is an Mimic Proxy
    let fooErrorInstanceESValue = ESValue.wrap(fooErrorInstance);
    fooErrorInstanceESValue.setProperty(ESValue.wrap("barProperty"), ESValue.wrap(setValue));
    arktest.assertEQ(fooErrorInstanceESValue.getProperty("barProperty").toNumber(), setValue);

    // test for XrefObject is an JSValue
    let fooInstanceESValue = ESValue.wrap(fooInstance);
    fooInstanceESValue.setProperty(ESValue.wrap("barProperty"), ESValue.wrap(setValue));
    arktest.assertEQ(fooInstanceESValue.getProperty("barProperty").toNumber(), setValue);
}

function testXrefObjectIsInstanceOf() {
    // test for XrefObject is an Mimic Proxy
    arktest.assertTrue(fooErrorInstance instanceof FooError);
    arktest.assertTrue(fooErrorInstance instanceof Error);

    // test for XrefObject is an JSValue
    arktest.assertTrue(fooInstance instanceof Foo);
}

function testXrefObjectInvoke() {
    // functions are always to be Mimic Proxies
    arktest.assertEQ(Foo.getMagic(), 0x55aa);
}

function testXrefObjectInvokeMethod() {
    // functions are always to be Mimic Proxies
    arktest.assertEQ(fooInstance.getBar(), fooInstance.barProperty);
}

function testXrefHasProperty() {
    /* 
     * test for XrefObject is an Mimic Proxy
     */
    let fooErrorInstanceESValue = ESValue.wrap(fooErrorInstance);
    let arrayMimicESValue = ESValue.wrap(fooErrorInstance.arrayProperty);

    arktest.assertTrue(fooErrorInstanceESValue.hasProperty("barProperty"));                  // by name
    arktest.assertTrue(fooErrorInstanceESValue.hasProperty(ESValue.wrap("barProperty")));    // by value
    arktest.assertFalse(arrayMimicESValue.getPropertySafe(1).isUndefined());                 // by index, no hasPropertyByIndex, test through getPropertySafe

    arktest.assertFalse(fooErrorInstanceESValue.hasProperty("nonExistentProperty"));                // by name no exists
    arktest.assertFalse(fooErrorInstanceESValue.hasProperty(ESValue.wrap("nonExistentProperty")));  // by value no exists
    arktest.assertTrue(arrayMimicESValue.getPropertySafe(100).isUndefined());                       // by index no exists, same reason as above

    arktest.assertTrue(fooErrorInstanceESValue.hasOwnProperty("arrayProperty"));                  // by name, ownProperty is true
    arktest.assertTrue(fooErrorInstanceESValue.hasProperty("message"));                           // by name, ownProperty is false, message is inherited from Error
    arktest.assertTrue(fooErrorInstanceESValue.hasOwnProperty("message"));                        // by name, ownProperty is true
    
    arktest.assertTrue(fooErrorInstanceESValue.hasProperty(ESValue.wrap("arrayProperty")));       // by value, ownProperty is true
    arktest.assertTrue(fooErrorInstanceESValue.hasProperty(ESValue.wrap("message")));             // by value, ownProperty is false, message is inherited from Error
    arktest.assertTrue(fooErrorInstanceESValue.hasOwnProperty(ESValue.wrap("message")));          // by value, ownProperty is true

    /* 
     * test for XrefObject is an JSValue
     */
    let fooInstanceESValue = ESValue.wrap(fooInstance);
    let arrayLikeJSValueESValue = ESValue.wrap(fooInstance.arrayLikeProperty);

    arktest.assertTrue(fooInstanceESValue.hasProperty("barProperty"));                      // by name
    arktest.assertTrue(fooInstanceESValue.hasProperty(ESValue.wrap("barProperty")));        // by value
    arktest.assertFalse(arrayLikeJSValueESValue.getPropertySafe(1).isUndefined());          // by index, same reason as above

    arktest.assertFalse(fooInstanceESValue.hasProperty("nonExistentProperty"));                     // by name no exists
    arktest.assertFalse(fooInstanceESValue.hasProperty(ESValue.wrap("nonExistentProperty")));       // by value no exists
    arktest.assertTrue(arrayLikeJSValueESValue.getPropertySafe(100).isUndefined());                 // by index no exists, same reason as above
}

function testXrefObjectInstantiate() {
    let foo = new Foo();
    arktest.assertTrue(foo instanceof Foo);
    arktest.assertTrue(foo.barProperty === 0x55aa);
}

function testXrefObjectStrictlyEqual() {
    // test for XrefObject is an Mimic Proxy
    arktest.assertTrue(fooErrorInstance === fooErrorInstance);
    let newFooErrorInstance = new FooError();
    arktest.assertFalse(fooErrorInstance === newFooErrorInstance);

    // test for XrefObject is an JSValue
    arktest.assertTrue(fooInstance === fooInstance);
    let newFooInstance = new Foo();
    arktest.assertFalse(fooInstance === newFooInstance);
}

function testXrefObjectIsTrue() {
    // test for XrefObject is an Mimic Proxy
    if(fooErrorInstance) {
        arktest.assertTrue(true);
    } else {
        arktest.assertTrue(false);
    }

    // test for XrefObject is an JSValue
    if(fooInstance) {
        arktest.assertTrue(true);
    } else {
        arktest.assertTrue(false);
    }
}

function testXrefObjectTypeof() {
    // test for XrefObject is an Mimic Proxy
    arktest.assertEQ(typeof fooErrorInstance, "object");
    arktest.assertEQ(typeof fooErrorInstance.arrayProperty, "object");

    // test for XrefObject is an JSValue
    arktest.assertEQ(typeof fooInstance, "object");
    arktest.assertEQ(typeof fooInstance.arrayLikeProperty, "object");
}

function testAll() {
    testXrefObjectAll();
}