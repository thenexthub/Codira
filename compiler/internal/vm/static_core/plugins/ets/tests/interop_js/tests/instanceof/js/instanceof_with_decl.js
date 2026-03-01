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
let A = etsVm.getClass('Linstanceof/A;');

export class AValue {}
export class ADeclared {}
export class BDeclared {}
export class CValue extends BDeclared {}
export class CDeclared extends ADeclared {}
export class DDeclared extends A {}

export function asJsvalue(x) {
	return x;
}

export function jsFn() {
	return new ADeclared();
}

export const jsAvalue = new AValue();
export const jsCvalue = new CValue();
export const createError = (msg) => {return new Error(msg)};
