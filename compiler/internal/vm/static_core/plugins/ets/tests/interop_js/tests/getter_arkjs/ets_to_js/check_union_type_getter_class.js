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

import { string, number} from './constant';

const etsVm = globalThis.gtest.etsVm;
const UnionTypeClass = etsVm.getClass('Lgetter/test/UnionTypeClass;');
const createUnionTypeGetterClassFromEts = etsVm.getFunction('Lgetter/test/ETSGLOBAL;', 'create_union_type_getter_class_from_ets');


function checkUnionTypeGetterClassValueString() {
	const GClass = new UnionTypeClass(string);
	ASSERT_TRUE(GClass.value === string);
}

function checkUnionTypeGetterClassValueNumber() {
	const GClass = new UnionTypeClass(number);
	ASSERT_TRUE(GClass.value === number);
}

function checkCreateUnionTypeGetterClassFromEtsValueString() {
	const GClass = createUnionTypeGetterClassFromEts(string);
	ASSERT_TRUE(GClass.value === string);
}

function checkCreateUnionTypeGetterClassFromEtsValueNumber() {
	const GClass = createUnionTypeGetterClassFromEts(number);
	ASSERT_TRUE(GClass.value === number);
}


checkUnionTypeGetterClassValueString();
checkUnionTypeGetterClassValueNumber();
checkCreateUnionTypeGetterClassFromEtsValueString();
checkCreateUnionTypeGetterClassFromEtsValueNumber();