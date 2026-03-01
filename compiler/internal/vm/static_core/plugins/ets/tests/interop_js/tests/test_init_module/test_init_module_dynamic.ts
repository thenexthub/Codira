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

let etsVm = globalThis.gtest.etsVm;

export let initModuleTest = {
    initMagic: `MODULE_INITED`,
    isInited: `NOT_INITED`
};

export let shouldNotInitClassTest = {
    initMagic: `CLASS_INITED`,
    isInited: `NOT_INITED`,
};

// Ensure that the top level statements are 
// executed when getting the module class
function testInitModule(): void {
    ASSERT_TRUE(initModuleTest.isInited != initModuleTest.initMagic);

    // get ETSGLOBAL class will init the module
    let etsGlobal = etsVm.getClass('Ltest_init_module_static/ETSGLOBAL;');
    ASSERT_TRUE(initModuleTest.isInited === initModuleTest.initMagic);
}

// Ensure that the top level statements are 
// not executed when getting the not-module class
function testShouldNotInitModule(): void {
    ASSERT_TRUE(shouldNotInitClassTest.isInited != shouldNotInitClassTest.initMagic);

    // get SomeClass class will not init the class
    let SomeClass = etsVm.getClass('Ltest_init_module_static/SomeClass;');
    ASSERT_TRUE(shouldNotInitClassTest.isInited != shouldNotInitClassTest.initMagic);

    // new instance of SomeClass will init the class
    let instance = new SomeClass();
    ASSERT_TRUE(instance != undefined);
    ASSERT_TRUE(shouldNotInitClassTest.isInited === shouldNotInitClassTest.initMagic);
}

function main(): void {
    testInitModule();
    testShouldNotInitModule();
}

main();
