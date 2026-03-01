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


const etsVm = globalThis.gtest.etsVm;

const mod = etsVm.getModule('get_module');
ASSERT_TRUE(mod !== undefined);
ASSERT_TRUE(typeof mod.b === 'number' && mod.b === 100);
ASSERT_TRUE(typeof mod.c === 'function' && mod.c() === 'hello from c');
ASSERT_TRUE(typeof mod.A === 'function');
const a1 = new mod.A();
ASSERT_TRUE(typeof a1.value === 'function' && a1.value() === 42);
ASSERT_TRUE(mod.D === undefined);

const entry = etsVm.getModule('entry_module');
ASSERT_TRUE(entry !== undefined);
ASSERT_TRUE(typeof entry.b === 'number' && entry.b === 100);       
ASSERT_TRUE(typeof entry.c === 'function' && entry.c() === 'hello from c');
ASSERT_TRUE(typeof entry.A === 'function');                          
const a2 = new entry.A();
ASSERT_TRUE(typeof a2.value === 'function' && a2.value() === 42);
ASSERT_TRUE(entry.D === undefined);


const entry2 = etsVm.getModule('entry_module2');
ASSERT_TRUE(entry2 !== undefined);
ASSERT_TRUE(typeof entry2.b === 'number' && entry2.b === 100);   
ASSERT_TRUE(typeof entry2.c === 'function' && entry2.c() === 'hello from c');
ASSERT_TRUE(typeof entry2.A === 'function');                          
const a3 = new entry2.A();
ASSERT_TRUE(typeof a3.value === 'function' && a3.value() === 42);
ASSERT_TRUE(entry2.D === undefined);

function testLoadNotFoundModule() {
    let result = false;
    try {
        etsVm.getModule('not_module');
    } catch(e) {
        result = e.toString().startsWith('LinkerClassNotFoundError')
                 && e.message.startsWith('Cannot find class Lnot_module/ETSGLOBAL;');
    }
    ASSERT_TRUE(result);
}

testLoadNotFoundModule();