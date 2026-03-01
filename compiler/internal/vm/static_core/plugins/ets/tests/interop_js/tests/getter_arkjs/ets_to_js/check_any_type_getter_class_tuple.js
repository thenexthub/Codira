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

import { tuple } from './constant';

const etsVm = globalThis.gtest.etsVm;
const checkObj = (arg) => arg !== null && typeof arg === 'object' && !Array.isArray(arg);
const AnyTypeClass = etsVm.getClass('Lgetter/test/AnyTypeClass;');

const createAnyTypeGetterClassFromEtsTuple = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_any_type_getter_class_from_ets_tuple');

function checkClassGetterClassValueTuple() {
    const GClass = new AnyTypeClass(tuple);
    const res = GClass.value;
    ASSERT_TRUE(checkObj(res) && res.$0 === tuple[0] && res.$1 === tuple[1]);
}

function checkCreateAnyTypeGetterClassFromEtsTuple() {
    const GClass = createAnyTypeGetterClassFromEtsTuple(tuple);
    const res = GClass.value;
    ASSERT_TRUE(checkArray(res) && res[0] === tuple[0] && res[1] === tuple[1]);
}

checkClassGetterClassValueTuple();
checkCreateAnyTypeGetterClassFromEtsTuple();
