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

'use strict';

export function bouncingPandas() {
	const Vec2 = gtest.etsVm.getClass('Lbouncing_pandas/Vec2;');

	const vec = new Vec2(1200, 800);
	const countSB = 4;

	const SBody = gtest.etsVm.getClass('Lbouncing_pandas/SBody;');

	const arrSBody = [];
	const step = 13;
	for (let i = 0; i < countSB; ++i) {
		let el = new SBody();
		el.rad = 11;
		el.m = 11 * 11;
		el.r.x = (step * i) % 1200;
		el.r.y = Math.round(step * ((step * i) / 1200 + 1));
		el.v.x = 5;
		el.v.y = 5;

		arrSBody[i] = el;
	}
	let recomputeFrameSBody = gtest.etsVm.getFunction('Lbouncing_pandas/ETSGLOBAL;', 'recomputeFrameSBody');
	const resArr = recomputeFrameSBody(arrSBody, vec);

	let resultHash = 0;
	for (let i = 0; i < countSB; ++i) {
		resultHash += resArr[i].r.x + resArr[i].r.y + resArr[i].v.x + resArr[i].v.y + resArr[i].rad + resArr[i].m;
	}

	if (Math.round(resultHash) !== 734) {
		console.log('Wrong result hash ' + Math.round(resultHash));
		return 1;
	} else {
		return 0;
	}
}

ASSERT_TRUE(bouncingPandas() === 0);
