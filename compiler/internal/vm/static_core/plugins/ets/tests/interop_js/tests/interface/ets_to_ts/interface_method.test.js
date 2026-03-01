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

const num = 1;
const string = 'string';
const bool = true;
const arr = [];
const obj = {};

function isObject(variable) {
	return variable !== null && typeof variable === 'object' && !Array.isArray(variable);
}

// NOTE #1835 #17769 set "true" when literal types and optional methods are fully fixed
const FIXES_IMPLEMENTED = false;

const MODULE_PATH = 'Linterface_method/test/ETSGLOBAL;';

const AnyTypeMethodClass = etsVm.getClass('Linterface_method/test/AnyTypeMethodClass;');
const createInterfaceClassAnyTypeMethod = etsVm.getFunction(MODULE_PATH, 'create_interface_class_any_type_method');

const UnionTypeMethodClass = etsVm.getClass('Linterface_method/test/UnionTypeMethodClass;');
const createInterfaceClassUnionTypeMethod = etsVm.getFunction(MODULE_PATH, 'create_interface_class_union_type_method');

const TupleTypeMethodClass = etsVm.getClass('Linterface_method/test/TupleTypeMethodClass;');
const createInterfaceClassTupleTypeMethod = etsVm.getFunction(MODULE_PATH, 'create_interface_class_tuple_type_method');

const SubsetByRefClass = etsVm.getClass('Linterface_method/test/SubsetByRefClass;');
const subsetByRefInterface = etsVm.getFunction('Linterface_method/test/ETSGLOBAL;', 'subset_by_ref_interface');
const callSubsetByRefInterfaceFromEts = etsVm.getFunction(MODULE_PATH, 'call_subset_by_ref_interface_from_ets');

const SubsetByValueClass = etsVm.getClass('Linterface_method/test/SubsetByValueClass;');
const createSubsetByValueClassFromEts = etsVm.getFunction(MODULE_PATH, 'create_subset_by_value_class_from_ets');

const getExtras = () => ({
	WithOptionalMethodClass: etsVm.getClass('Linterface_method/test/WithOptionalMethodClass;'),
	createClassWithOptionalMethod: etsVm.getFunction('Linterface_method/test/ETSGLOBAL;', 'create_class_with_optional_method'),
	WithoutOptionalMethodClass: etsVm.getClass('Linterface_method/test/TupleClass;'),
	createClassWOOptionals: getFunction('Linterface_method/test/ETSGLOBAL;', 'create_class_without_optional_method'),
	optionalArg: etsVm.getFunction('Linterface_method/test/ETSGLOBAL;', 'optional_arg'),
	optionalArgArray: etsVm.getFunction('Linterface_method/test/ETSGLOBAL;', 'optional_arg_array'),
	LiteralValueMethodClass: etsVm.getClass('Linterface_method/test/LiteralValueMethodClass;'),
	etsFnInterfaceLiteralValueClass: 'create_interface_literal_value_class_from_ets',
	createInterfaceLiteralValueClassFromEts: etsVm.getFunction(MODULE_PATH, etsFnInterfaceLiteralValueClass),
});

const exposedValues = {
	num,
	string,
	bool,
	arr,
	obj,
	isObject,
	AnyTypeMethodClass,
	createInterfaceClassAnyTypeMethod,
	UnionTypeMethodClass,
	createInterfaceClassUnionTypeMethod,
	SubsetByRefClass,
	subsetByRefInterface,
	callSubsetByRefInterfaceFromEts,
	SubsetByValueClass,
	createSubsetByValueClassFromEts,
	TupleTypeMethodClass,
	createInterfaceClassTupleTypeMethod,
};

module.exports = Object.assign(exposedValues, FIXES_IMPLEMENTED ? getExtras() : {});
